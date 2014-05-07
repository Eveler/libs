#include "morganizationdatasource.h"

#include "morganizationdbwrapper.h"

#include <QDebug>


/*
 * Begin C++ - QML class definition: *[ MOrganizationDataSource ]*
*/
MOrganizationDataSource::MOrganizationDataSource(QObject *parent) :
  MAbstractDataSource(new MOrganizationDBWrapper(), parent)
{
}

MOrganizationDataSource::~MOrganizationDataSource()
{
  //  qDebug() << __func__ << __LINE__;
  MAbstractDBWrapper *wrapper = dbWrapper();
  setDBWrapper( NULL );

  //  qDebug() << __func__ << __LINE__ << wrapper;
  delete wrapper;
  wrapper = NULL;
}

void MOrganizationDataSource::findObjectFinished()
{
  disconnect( dbWrapper(), SIGNAL(finished()), this, SLOT(findObjectFinished()) );

  founded()->resetModel();

  dbWrapper()->releaseOldResources();

  emit statusChanged();
}

void MOrganizationDataSource::initiateObjectFinished()
{
//  disconnect( dbWrapper(), SIGNAL(finished()), this, SLOT(initiateObjectFinished()) );

//  int index = dbWrapper()->count( (int)MAbstractDBWrapper::Initiated )-1;
//  initiated()->insertObjects( index, index );

//  dbWrapper()->releaseOldResources();

  emit statusChanged();
}

void MOrganizationDataSource::saveObjectFinished()
{
//  disconnect( dbWrapper(), SIGNAL(finished()), this, SLOT(saveObjectFinished()) );

//  int index = dbWrapper()->count( MAbstractDBWrapper::Selected )-1;
//  selected()->insertObjects( index, index );

//  index = savedObjectIndex();
//  setSavedObjectIndex( -1 );
//  initiated()->removeObjects( index, index );

//  dbWrapper()->releaseOldResources();

  emit saved();
  emit statusChanged();
}
/*
 * End class definition: *[ MOrganizationDataSource ]*
*/