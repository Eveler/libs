#include "humanloader.h"

#include "humanloader_p.h"

#include <QSqlDatabase>


HumanLoader::HumanLoader(QObject *parent) :
  QObject(parent)
{
  p = new HumanLoader_P( this );
  connect( p, SIGNAL(sendError(QString)), SLOT(receivedError(QString)) );
  connect( p, SIGNAL(availableCountChanged()), SIGNAL(countChanged()) );
  connect( p, SIGNAL(started()), SLOT(threadStarted()) );
  connect( p, SIGNAL(finished()), SLOT(threadFinished()) );
  loop = new QEventLoop( this );
}

HumanLoader::~HumanLoader()
{
  p->m__LastError.clear();
  delete p;
  p = NULL;
}

QString HumanLoader::lastError() const
{
  return p->m__LastError;
}

QString HumanLoader::connectionName()
{
  if ( p->m__ConnectionName.isEmpty() && !QSqlDatabase::connectionNames().isEmpty() )
  {
    p->m__ConnectionName = QSqlDatabase::connectionNames().first();
    emit connectionNameChanged();
  }
  return p->m__ConnectionName;
}

bool HumanLoader::setConnectionName( QString connectionName )
{
  if ( !QSqlDatabase::contains( connectionName ) )
  {
    QString connection = this->connectionName();
    if ( connection.isEmpty() ) return false;
  }
  else
  {
    p->m__ConnectionName = connectionName;
    emit connectionNameChanged();
  }
  return true;
}

bool HumanLoader::started() const
{
  return p->isRunning();
}

bool HumanLoader::load( QString filter, bool blockUI )
{
  if ( p->m__Started || p->m__AvailableCount != p->m__ReceivedCount )
  {
    receivedError( tr( "Процесс загрузки списка пользователей занят" ) );
    return false;
  }

  p->m__Filter = filter;
  p->start();
  if ( blockUI )
    return ( loop->exec() == 0 );
  else return true;
}

int HumanLoader::count() const
{
  return p->m__AvailableCount;
}

HumanInfo * HumanLoader::newInfo()
{
  return p->newInfo();
}

void HumanLoader::threadStarted()
{
  p->m__Started = true;
  emit startedChanged();
}

void HumanLoader::threadFinished()
{
  p->m__Started = false;
  loop->exit( ( p->m__Successfully ? 0 : 1 ) );
  emit startedChanged();
}

void HumanLoader::receivedError( QString errorText )
{
  p->m__LastError = errorText;
  emit lastErrorChanged();
}
