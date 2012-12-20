#include "structurestatusesstorage.h"

StructureStatusesStorage *StructureStatusesStorage::m__Instance = NULL;

StructureStatusesStorage * StructureStatusesStorage::instance()
{
  if ( m__Instance == NULL ) m__Instance = new StructureStatusesStorage;

  return m__Instance;
}

bool StructureStatusesStorage::setStorage( StorageItemModel *stor, StructStructureStatusCols cols )
{
  if ( m__Storage != NULL )
    storageDestroyed();

  if ( stor == NULL )
  {
    reset();

    return true;
  }

  if ( stor->findColumnByRealName( stor->getPropertiesView(), cols.Id ) == -1 ||
       stor->findColumnByRealName( stor->getPropertiesView(), cols.ShortName ) == -1 ||
       stor->findColumnByRealName( stor->getPropertiesView(), cols.FullName ) )
    return false;

  m__Storage = stor;
  m__Cols = cols;

  connect( stor, SIGNAL(destroyed()),
           this, SLOT(storageDestroyed()) );
  connect( stor, SIGNAL(recordAdded(MFCRecord*,int)),
           this, SLOT(recordAdded(MFCRecord*,int)) );
  connect( stor, SIGNAL(recordRemoved(MFCRecord*,int)),
           this, SLOT(recordRemoved(MFCRecord*,int)) );
  connect( stor, SIGNAL(recordRemoved(MFCRecord*,int)),
           this, SLOT(disconnectRecord(MFCRecord*,int)) );

  for ( int rIdx = 0; rIdx < stor->availableRecords().count(); rIdx++ )
  {
    MFCRecord *record = stor->availableRecords()[rIdx];
    connect( record, SIGNAL(propertyChanged(QString)),
             this, SLOT(propertyChanged(QString)) );
    recordAdded( record, rIdx );
  }

  return true;
}

StorageItemModel * StructureStatusesStorage::storage() const
{
  return m__Storage;
}

const QList<StructureStatus *> & StructureStatusesStorage::objects() const
{
  return m__StructureStatuses;
}

QList<StructureStatus *> StructureStatusesStorage::findById(
    QList<StructureStatus *> objs, QVariant value ) const
{
  QList<StructureStatus *> result = QList<StructureStatus *>();

  foreach ( StructureStatus *obj, objs )
    if ( obj->id() == value ) result << obj;

  return result;
}

StructureStatusesStorage::StructureStatusesStorage(QObject *parent) :
    QObject(parent)
{
  reset();
}

StructureStatusesStorage::~StructureStatusesStorage()
{
  reset();
}

void StructureStatusesStorage::reset()
{
  m__Storage = NULL;
  m__Cols.Id.clear();
  m__Cols.ShortName.clear();
  m__Cols.FullName.clear();
}

void StructureStatusesStorage::setObjectData( StructureStatus *obj, MFCRecord *record )
{
  obj->setId( record->currentProperty( m__Cols.Id ) );
  obj->setShortName( record->currentProperty( m__Cols.ShortName ).toString() );
  obj->setFullName( record->currentProperty( m__Cols.FullName ).toString() );
}

void StructureStatusesStorage::storageDestroyed()
{
  disconnect( this, SLOT(storageDestroyed()) );
  disconnect( this, SLOT(recordAdded(MFCRecord*,int)) );
  disconnect( this, SLOT(recordRemoved(MFCRecord*,int)) );
  disconnect( this, SLOT(propertyChanged(QString,QVariant)) );
}

void StructureStatusesStorage::recordAdded( MFCRecord *record, int index )
{
  StructureStatus *structureStatus = new StructureStatus( this );
  setObjectData( structureStatus, record );
  m__StructureStatuses.insert( index, structureStatus );
}

void StructureStatusesStorage::recordRemoved( MFCRecord */*record*/, int index )
{
  m__StructureStatuses.removeAt( index );
}

void StructureStatusesStorage::disconnectRecord( MFCRecord *record, int )
{
  disconnect( record, SIGNAL(propertyChanged(QString)),
              this, SLOT(propertyChanged(QString)) );
}

void StructureStatusesStorage::propertyChanged( QString column )
{
  if ( column != m__Cols.Id && column != m__Cols.ShortName &&
       column != m__Cols.FullName )
    return;

  MFCRecord *record = qobject_cast<MFCRecord *>( sender() );
  int index = m__Storage->availableRecords().indexOf( record );
  StructureStatus *obj = objects()[index];
  setObjectData( obj, record );
}
