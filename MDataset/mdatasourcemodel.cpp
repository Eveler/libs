#include "mdatasourcemodel.h"


/*
 * Begin C++ class definition: *[ ObjectListPrivate ]*
*/
ObjectListPrivate::ObjectListPrivate() {}

ObjectListPrivate::~ObjectListPrivate()
{
  while( !m__Objects.isEmpty() )
    m__Objects.take( m__Objects.keys().first() ).clear();
  while( !m__StarObjects.isEmpty() )
    m__StarObjects.take( m__StarObjects.keys().first() ).clear();
}

QObject * ObjectListPrivate::pObject( int sourceType, int index ) const
{
  return m__Objects.value( sourceType, QObjectList() ).value( index, NULL );
}

int ObjectListPrivate::pCount( int sourceType ) const
{
  return m__Objects.value( sourceType, QObjectList() ).count();
}

int ObjectListPrivate::pIndex( int sourceType, QObject *object ) const
{
  return m__Objects.value( sourceType, QObjectList() ).indexOf( object );
}

void ObjectListPrivate::pAppend( int sourceType, QObject *object )
{
  if ( !m__Objects.contains( sourceType ) ) m__Objects[sourceType] = QObjectList();

  m__Objects[sourceType] << object;
}

QObject * ObjectListPrivate::pTake( int sourceType, int index )
{
  return m__Objects[sourceType].takeAt( index );
}

QObject * ObjectListPrivate::pObject( void *sourceType, int index ) const
{
  return m__StarObjects.value( sourceType, QObjectList() ).value( index, NULL );
}

int ObjectListPrivate::pCount( void *sourceType ) const
{
  return m__StarObjects.value( sourceType, QObjectList() ).count();
}

int ObjectListPrivate::pIndex( void *sourceType, QObject *object ) const
{
  return m__StarObjects.value( sourceType, QObjectList() ).indexOf( object );
}

void ObjectListPrivate::pAppend( void *sourceType, QObject *object )
{
  if ( !m__StarObjects.contains( sourceType ) ) m__StarObjects[sourceType] = QObjectList();

  m__StarObjects[sourceType] << object;
}

QObject * ObjectListPrivate::pTake( void *sourceType, int index )
{
  return m__StarObjects[sourceType].takeAt( index );
}
/*
 * End class definition: *[ ObjectListPrivate ]*
*/


/*
 * Begin C++ - QML class definition: *[ MDataSourceModel ]*
*/
MDataSourceModel::MDataSourceModel(QObject *parent) :
  MObjectModel(parent),
  m__Source(NULL),
  m__SourceType(-1)
{
}

MDataSourceModel::~MDataSourceModel()
{
  m__Source = NULL;
}

int MDataSourceModel::rowCount( const QModelIndex &index ) const
{
  Q_UNUSED(index)

  int result = 0;
  if ( m__Source == NULL ) return result;

  if ( m__SourceType == -1 ) result = m__Source->count( (void *)this );
  else result = m__Source->count( m__SourceType );

  return result;
}

QVariant MDataSourceModel::data( const QModelIndex &index, int role ) const
{
  Q_UNUSED(role)
  if ( m__Source == NULL || !index.isValid() || index.row() < 0 ) return QVariant();

  QObject *object = NULL;

  if ( m__SourceType == -1 )
  {
    if ( index.row() >= m__Source->count( (void *)this ) ) return QVariant();

    object = m__Source->object( (void *)this, index.row() );
  }
  else
  {
    if ( index.row() >= m__Source->count( m__SourceType ) ) return QVariant();

    object = m__Source->object( m__SourceType, index.row() );
  }

  return QVariant::fromValue( object );
}

SafelyValue * MDataSourceModel::get( int index ) const
{
  if ( m__Source == NULL || index < 0 )
  {
    qDebug() << __func__ << index;
    return NULL;
  }

  SafelyValue *result = NULL;
  if ( m__SourceType == -1 )
  {
    if ( index >= m__Source->count( (void *)this ) )
    {
      qDebug() << __func__ << index;
      return NULL;
    }

    result = new SafelyValue( m__Source->object( (void *)this, index ) );
  }
  else
  {
    if ( index >= m__Source->count( m__SourceType ) )
    {
      qDebug() << __func__ << index;
      return NULL;
    }

    result = new SafelyValue( m__Source->object( m__SourceType, index ) );
  }

  result->deleteLater();
  return result;
}

void MDataSourceModel::append( QObject *object )
{
  Q_UNUSED(object)
}

void MDataSourceModel::insert( int index, QObject *object )
{
  Q_UNUSED(index)
  Q_UNUSED(object)
}

void MDataSourceModel::remove( int index )
{
  Q_UNUSED(index)
}

void MDataSourceModel::replace( int index, QObject *object )
{
  Q_UNUSED(index)
  Q_UNUSED(object)
}

int MDataSourceModel::count() const
{
  int result = 0;
  if ( m__Source == NULL ) return result;

  if ( m__SourceType == -1 ) result = m__Source->count( (void *)this );
  else result = m__Source->count( m__SourceType );

  return result;
}

int MDataSourceModel::index( QObject *object ) const
{
  if ( m__Source == NULL ) return -1;

  if ( m__SourceType == -1 ) return m__Source->index( (void *)this, object );
  else return m__Source->index( m__SourceType, object );
}

void MDataSourceModel::setSource( ObjectListPrivate *source )
{
  m__Source = source;
  resetModel();
}

int MDataSourceModel::sourceType() const
{
  return m__SourceType;
}

void MDataSourceModel::setSourceType( int sourceType )
{
  m__SourceType = sourceType;
  emit sourceTypeChanged();
}

void MDataSourceModel::resetModel()
{
  beginResetModel();
  endResetModel();
}

void MDataSourceModel::insertObjects( int first, int last )
{
  beginInsertRows( QModelIndex(), first, last );
  endInsertRows();
}

void MDataSourceModel::removeObjects( int first, int last )
{
  beginRemoveRows( QModelIndex(), first, last );
  endRemoveRows();
}
/*
 * End class definition: *[ MDataSourceModel ]*
*/