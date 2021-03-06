#include "mcalculationalcolumn.h"

#include "mcalculationalcolumnprivate.h"
#include "mcalculationalmodel.h"
#include "mcalculationalrow.h"

#include <QDebug>


int MCalculationalColumn::sectionCount() const
{
  return p->m__Data.count();
}

QVariant MCalculationalColumn::label() const
{
  return p->m__Label;
}

void MCalculationalColumn::setLabel( QVariant label )
{
  QVariant oldValue = p->m__Label;
  p->m__Label = label;

  emit labelChanged( oldValue, label );
}

QVariant MCalculationalColumn::data( int row ) const
{
  if ( row < 0 || row >= p->m__Data.count() ) return QVariant();

  return p->m__Data[row];
}

bool MCalculationalColumn::setData( int row, const QVariant &value )
{
  if ( row < 0 || row >= p->m__Data.count() ) return false;

  if ( algorithmForRow( row ) != NULL ) return false;
  if ( p->m__Model->row( row )->algorithmForColumn( column() ) != NULL ) return false;

  setDataPrivate( row, value );

  return true;
}

MCalculationalModel * MCalculationalColumn::model() const
{
  return p->m__Model;
}

int MCalculationalColumn::column()
{
  return p->m__Model->findColumn( this );
}

const QList<MAbstractColumnCalculationAlgorithm *> &
MCalculationalColumn::columnAlgorithms() const
{
  return p->m__ColumnAlgorithms;
}

MAbstractColumnCalculationAlgorithm *
MCalculationalColumn::algorithmForRow( int row ) const
{
  foreach ( MAbstractColumnCalculationAlgorithm *algorithm, p->m__ColumnAlgorithms )
    if ( algorithm->servedRows().contains( row ) ) return algorithm;

  return NULL;
}

MCalculationalColumn::MCalculationalColumn( MCalculationalModel *model ) :
  QObject(model)
{
  p = new MCalculationalColumnPrivate( model );
}

MCalculationalColumn::~MCalculationalColumn()
{
  delete p;
  p = NULL;
}

void MCalculationalColumn::insert( int row )
{
  p->m__Data.insert( row, QVariant() );
}

void MCalculationalColumn::remove( int row )
{
  p->m__Data.removeAt( row );
}

void MCalculationalColumn::addColumnAlgorithm( MAbstractColumnCalculationAlgorithm *algorithm )
{
  p->m__ColumnAlgorithms << algorithm;
}

void MCalculationalColumn::setDataPrivate( int row, QVariant value )
{
  QVariant oldValue = p->m__Data[row];
  if ( oldValue == value ) return;
  p->m__Data[row] = value;

  emit dataChanged( row, oldValue, value );
}
