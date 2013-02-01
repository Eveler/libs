#include "amslogger5.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QTextCodec>

bool AMSLogger::initialized=false;
bool AMSLogger::installed=false;
AMSLogger::LogLevels AMSLogger::loglevel=AMSLogger::LevelCritical;
QFile *AMSLogger::outFile=new QFile();
QtMessageHandler AMSLogger::oldMsgHandler=NULL;
int AMSLogger::rotateCount=14;
QTextStream AMSLogger::stream(stderr);
int AMSLogger::oldMsgType=-1;

void AMSLogger::messageOutput(QtMsgType type, const QMessageLogContext &,const QString &msg){
  AMSLogger::initialyze();
  QByteArray ba(qPrintable(msg));
  QString strDateTime=
      QDateTime::currentDateTime().toString("[dd.MM.yyyy] [hh:mm:ss.zzz]: ");

  switch (type){
  case QtDebugMsg:
    if(oldMsgType!=type){
      if(stream.device()->pos()>0) stream<<endl;
      ba.prepend("Debug: ");
      ba.prepend(qPrintable(strDateTime));
    }
    stream<<ba;
    if(AMSLogger::logLevel().testFlag(AMSLogger::LevelDebug))
      if(outFile->fileName().length()>0) writeToFile(ba);
    break;

  case QtWarningMsg:
    if(oldMsgType!=type){
      if(stream.device()->pos()>0) stream<<endl;
      ba.prepend("Warning: ");
      ba.prepend(qPrintable(strDateTime));
    }
    stream<<msg;
    if(AMSLogger::logLevel().testFlag(AMSLogger::LevelWarn))
      if(outFile->fileName().length()>0) writeToFile(ba);
    break;

  case QtCriticalMsg:
    if(oldMsgType!=type){
      if(stream.device()->pos()>0) stream<<endl;
      ba.prepend("Critical: ");
      ba.prepend(qPrintable(strDateTime));
    }
    stream<<msg;
    if(AMSLogger::logLevel().testFlag(AMSLogger::LevelCritical))
      if(outFile->fileName().length()>0) writeToFile(ba);
    break;

  case QtFatalMsg:
    if(oldMsgType!=type){
      if(stream.device()->pos()>0) stream<<endl;
      ba.prepend("Fatal: ");
      ba.prepend(qPrintable(strDateTime));
    }
    stream<<msg;
    if(AMSLogger::logLevel().testFlag(AMSLogger::LevelFatal))
      if(outFile->fileName().length()>0) writeToFile(ba);
//    abort();
    break;
  }
  stream.flush();
  oldMsgType=type;
}

void AMSLogger::install(){
  initialyze();
  oldMsgHandler=qInstallMessageHandler(messageOutput);
  installed=true;
}

void AMSLogger::initialyze(){
  if(initialized) return;
//  loglevel=LevelCritical;
  QString logFile=prefix()+".log";
  outFile->setFileName(logFile);
  //"вращение" либо отправка по почте журнала//////////////////////////////////
  rotate();
  fprintf(stdout, "AMSLogger: logfile name: %s", qPrintable( outFile->fileName() ) );
  ////////////////////////////////////"вращение" либо отправка по почте журнала
  initialized=true;
}

void AMSLogger::writeToFile(const QByteArray &msg){
  QDir d=QDir();
  d.mkpath(QFileInfo(*outFile).absolutePath());
  if(!outFile->open(QFile::Append | QFile::WriteOnly | QFile::Text)){
    fprintf(stderr, "Warning: Error openning log file %s\n",
            qPrintable(outFile->fileName()));
    outFile->close();
    return;
  }

  QTextCodec *cfcs=QTextCodec::codecForUtfText(msg,QTextCodec::codecForLocale());
  if(cfcs){
    QString intenalMsg=cfcs->toUnicode(msg);
    QTextCodec *cfl=QTextCodec::codecForLocale();
    if(cfl && cfl->canEncode(intenalMsg))
      outFile->write(cfl->fromUnicode(intenalMsg));
    else outFile->write(msg);
  }else outFile->write(msg);

  outFile->close();
}

AMSLogger& AMSLogger::operator <<(const QVariant &msg){
  QString str;
  if(oldMsgType!=msgType){
    if(!msgFile.isEmpty()) str+=msgFile;
    if(msgLine>0 && !str.isEmpty()) str+=" ("+QVariant(msgLine).toString()+")";
    if(!str.isEmpty()) str+=": ";
  }
  str+=msg.toString();
  QMessageLogContext c;
  messageOutput(msgType,c,qPrintable(str));
  return *this;
}

AMSLogger& AMSLogger::operator <<(const void * ptr){
  QString str;
  QTextStream ts(&str);
  ts<<ptr;
  return (*this)<<str;
}

void AMSLogger::rotate(){
  if(outFile->fileName().isEmpty()) return;
  QString completeBN=completeBaseName();
  QString pfix=prefix();
  if(outFile->exists()){
    for(int i=rotateCount;i>0;i--){
      QFile::rename(pfix+"_"+QString::number(i)+".log",
                    pfix+"_"+QString::number(i+1)+".log");
    }
    QString pref=pfix;
    pref.chop(completeBN.length());
    QDir d=QDir(pref);
    d.setNameFilters(QStringList()<<(completeBN+"_*.log"));
    QStringList filesList=d.entryList(QDir::Files,QDir::Name);
    foreach(QString file,filesList){
      QString f=file;
      if(f.remove(completeBN+"_").remove(".log").toInt()>rotateCount){
        QFile::remove(d.absolutePath()+"/"+file);
        qDebug()<<file<<"removed";
      }
    }
    outFile->rename(pfix+"_1.log");
    outFile->setFileName(pfix+".log");
  }
}

QString AMSLogger::completeBaseName(){
  return QFileInfo(
        qApp->applicationFilePath()).completeBaseName();
}

QString AMSLogger::prefix(){
  QString completeBN=completeBaseName();
  return qApp->applicationDirPath()+"/"+completeBN+
      "_logger/"+completeBN;
}

AMSLOGGER_EXPORT AMSLogger logDebug(QString file,int line) {
  return AMSLogger(QtDebugMsg,file,line);
}
AMSLOGGER_EXPORT AMSLogger logWarning(QString file,int line) {
  return AMSLogger(QtWarningMsg,file,line);
}
AMSLOGGER_EXPORT AMSLogger logCritical(QString file,int line) {
  return AMSLogger(QtCriticalMsg,file,line);
}
AMSLOGGER_EXPORT AMSLogger logFatal(QString file,int line) {
  return AMSLogger(QtFatalMsg,file,line);
}