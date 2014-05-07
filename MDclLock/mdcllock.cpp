#include "mdcllock.h"
#include "httpclient.h"
#include "amslogger.h"
#include "checkconnection.h"
#include <QJsonDocument>
#include <QUuid>

#define setError(str) set_error(str, __FILE__, __LINE__)

MDclLock *MDclLock::self = NULL;
HttpClient *MDclLock::client = NULL;
QString MDclLock::errStr = QString();
QString MDclLock::msg = QString();


MDclLock *MDclLock::instance(const QUrl &url)
{
  if(!self) self = new MDclLock(url);
  return self;
}

bool MDclLock::lock(const int table_id, const QString &table_name,
                    const QString &user_name, const int priority)
{
  if(!checkSelf()) return true;

  QJsonArray params;
  params.append(table_id);
  params.append(table_name);
  params.append(user_name);
  params.append(priority);
  QJsonRpcMessage response = self->client->sendMessageBlocking(
        QJsonRpcMessage::createRequest("lock", params));

  if(response.type() == QJsonRpcMessage::Error){
    setError(tr("errorMessage = %1; errorData = %2; code = %3")
             .arg(response.errorMessage())
             .arg(response.errorData().toString())
             .arg(response.errorCode()));
    return true;
  }else{
    bool res = response.result().toBool();

    if(res){
      self->locks << qMakePair<int, QString>(table_id, table_name);
//      is_unlock_need(table_id, table_name);
    }else{
      msg = locked_by(table_id, table_name);
    }

    return res;
  }
}

void MDclLock::unlock(const int table_id, const QString &table_name)
{
  if(!checkSelf()) return;

  QJsonArray params;
  params.append(table_id);
  params.append(table_name);
  QJsonRpcMessage response = self->client->sendMessageBlocking(
        QJsonRpcMessage::createRequest("unlock", params));

  if(response.type() == QJsonRpcMessage::Error){
    setError(tr("errorMessage = %1; errorData = %2; code = %3")
             .arg(response.errorMessage())
             .arg(response.errorData().toString())
             .arg(response.errorCode()));
    return;
  }else{
//    for(int i=0;i<locks.size();i++){
//      QPair<int, QString> l = locks.at(i);
//      if(l.first==table_id && l.second==table_name) locks.removeAt(i);
//    }
    int i = self->locks.indexOf(qMakePair<int, QString>(table_id, table_name));
    if(i>-1) self->locks.removeAt(i);
    return;
  }
}

QString MDclLock::locked_by(const int table_id, const QString &table_name)
{
  if(!checkSelf()) return QString();

  QJsonArray params;
  params.append(table_id);
  params.append(table_name);
  QJsonRpcMessage response = self->client->sendMessageBlocking(
        QJsonRpcMessage::createRequest("locked_by", params));

  if(response.type() == QJsonRpcMessage::Error){
    setError(tr("errorMessage = %1; errorData = %2; code = %3")
             .arg(response.errorMessage())
             .arg(response.errorData().toString())
             .arg(response.errorCode()));
    return QString();
  }else{
    return response.result().toString();
  }
}

MDclLock *MDclLock::is_unlock_need(const int table_id,
                              const QString &table_name, const int priority)
{
  /// воспользуемся long poling для получения сообщения о необходимости
  /// разблокировать запись
  /// приэтом нужно соединить сигнал \fn unlockRequired() с методом снятия
  /// блокировки
  if(!checkSelf()) return NULL;

  QJsonArray params;
  params.append(table_id);
  params.append(table_name);
  params.append(priority);
  QJsonRpcServiceReply *reply = self->client->sendMessage(
        QJsonRpcMessage::createRequest("is_unlock_need", params));
  connect(reply, SIGNAL(finished()), self, SLOT(unlockRequested()));

  return self;
}

void MDclLock::setLogin(const QUrl &url, const QString &login,
                        const QString &pass)
{
  if(!self) instance();

  if(!self->client){
    self->client = new HttpClient(url.toString(), self);
    // register on server now
    self->registerOnServer();
    connect(qApp, SIGNAL(aboutToQuit()), self, SLOT(releaseClient()));
  }

  self->client->setEndPoint(url);
  self->client->setUsername(login);
  self->client->setPassword(pass);
}

MDclLock::~MDclLock()
{
//  LogDebug()<<Q_FUNC_INFO<<"BEGIN";
  errStr.clear();
  msg.clear();

  // отчистим все блокировки
  while(!locks.isEmpty()){
    QPair<int, QString> l = locks.first();
    LogWarning()<<"Unlock"<<l.second<<"("<<l.first<<")";
    unlock(l.first, l.second);
  }

  if(cc){
    delete cc;
  }

  if(client){
    delete client;
    client = NULL;
  }
  self = NULL;
//  LogDebug()<<Q_FUNC_INFO<<"END";
}

void MDclLock::release()
{
//  LogDebug()<<Q_FUNC_INFO<<"BEGIN";
  if(self){
    delete self;
    self = NULL;
  }
//  LogDebug()<<Q_FUNC_INFO<<"END";
}

void MDclLock::releaseClient()
{
//  LogDebug()<<Q_FUNC_INFO<<"BEGIN";
  if(client){
    delete client;
    client = NULL;
  }
//  LogDebug()<<Q_FUNC_INFO<<"END";
}

void MDclLock::unlockRequested()
{
  QJsonRpcServiceReply *reply = qobject_cast<QJsonRpcServiceReply*>(sender());
  if(!reply) return;

  QJsonDocument doc(reply->response().toObject());
  emit notification(doc);
//  LogDebug()<<doc.toJson();
  if(reply->response().result().isObject()){
    QJsonObject obj = reply->response().result().toObject();
//    LogDebug()<<"keys:"<<obj.keys().join("; ");
//    LogDebug()<<"table_name ="<<obj.value("table_name").toString()<<
//                "table_id ="<<obj.value("table_id").toInt()<<
//                "user ="<<obj.value("user").toString();
    emit unlockRequired();
    emit unlockRequired(obj.value("table_id").toInt(),
                        obj.value("table_name").toString(),
                        obj.value("user").toString());
    emit unlockRequired(obj.value("user").toString());
  }else if(reply->response().result().toBool()){
//    LogDebug()<<"emit unlockRequired()";
    emit unlockRequired();
    emit unlockRequired(tr("не определён"));
  }
  reply->deleteLater();
}

void MDclLock::errorRecieved(QString errStr)
{
  setError(errStr);
}

MDclLock::MDclLock(QObject *parent): QObject(parent),cc(NULL)
{
  connect(qApp, SIGNAL(aboutToQuit()), SLOT(release()));
}

MDclLock::MDclLock(const QUrl &url, QObject *parent):
  QObject(parent),cc(NULL)
{
  if(client) client->setEndPoint(url);
  else{
    client = new HttpClient(url.toString(), this);
    // register on server now
    registerOnServer();
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(releaseClient()));
  }
  connect(qApp, SIGNAL(aboutToQuit()), SLOT(release()));
}

void MDclLock::registerOnServer()
{
  QJsonRpcMessage response = client->sendMessageBlocking(
        QJsonRpcMessage::createRequest("register"));
  if(response.type() == QJsonRpcMessage::Error){
    setError(tr("errorMessage = %1; errorData = %2; code = %3")
             .arg(response.errorMessage())
             .arg(response.errorData().toString())
             .arg(response.errorCode()));
  }else{
    QString res = response.result().toString();

    if(!QUuid(res).isNull()){
      if(cc) delete cc;
      QUrl url = client->endPoint();
      cc = new CheckConnection(res, url.host(), url.port(9166)+1, this);
      connect(cc,SIGNAL(error(QString)),SLOT(errorRecieved(QString)));
      connect(cc,SIGNAL(disconnected()),SIGNAL(unlockRequired()));
    }
  }
}

bool MDclLock::checkSelf()
{
  if(!self){
    instance();
    LogWarning()<<"Need to setLogin";
    return false;
  }

  if(!self->client){
    LogWarning()<<"Need to setLogin";
    return false;
  }

  return true;
}

void MDclLock::set_error(const QString &str, const QString file, const int line)
{
  LogWarning()<<file<<tr("(")<<line<<tr("): ")<<str;
  errStr = str;
  if(self) emit self->error(str);
}