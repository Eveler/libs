#include "resultwayloader.h"

#include "resultwayloader_p.h"

#include <QSqlDatabase>


ResultwayLoader::ResultwayLoader(QObject *parent) :
  QObject(parent)
{
  p = new ResultwayLoader_P( this );
  connect( p, SIGNAL(sendError(QString)), SLOT(receivedError(QString)) );
  connect( p, SIGNAL(availableCountChanged()), SIGNAL(countChanged()) );
  connect( p, SIGNAL(started()), SLOT(threadStarted()) );
  connect( p, SIGNAL(finished()), SLOT(threadFinished()) );
  loop = new QEventLoop( this );
}

ResultwayLoader::~ResultwayLoader()
{
  p->m__LastError.clear();
  delete p;
  p = NULL;
}

QString ResultwayLoader::lastError() const
{
  return p->m__LastError;
}

QString ResultwayLoader::connectionName()
{
  if ( p->m__ConnectionName.isEmpty() && !QSqlDatabase::connectionNames().isEmpty() )
  {
    p->m__ConnectionName = QSqlDatabase::connectionNames().first();
    emit connectionNameChanged();
  }
  return p->m__ConnectionName;
}

bool ResultwayLoader::setConnectionName( QString connectionName )
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

bool ResultwayLoader::started() const
{
  return p->isRunning();
}

bool ResultwayLoader::load( QString filter, bool blockUI )
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

int ResultwayLoader::count() const
{
  return p->m__AvailableCount;
}

ResultwayInfo * ResultwayLoader::newInfo()
{
  return p->newInfo();
}

void ResultwayLoader::threadStarted()
{
  p->m__Started = true;
  emit startedChanged();
}

void ResultwayLoader::threadFinished()
{
  p->m__Started = false;
  loop->exit( ( p->m__Successfully ? 0 : 1 ) );
  emit startedChanged();
}

void ResultwayLoader::receivedError( QString errorText )
{
  p->m__LastError = errorText;
  emit lastErrorChanged();
}
