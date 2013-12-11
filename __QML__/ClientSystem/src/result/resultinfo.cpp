#include "resultinfo.h"


ResultInfo::ResultInfo() :
  m__Identifier(QVariant()),
  m__Name(QString())
{
}

ResultInfo::ResultInfo( const ResultInfo &other ) :
  m__Identifier(other.identifier()),
  m__Name(other.name())
{
}

ResultInfo::~ResultInfo()
{
}

QVariant ResultInfo::identifier() const
{
  return m__Identifier;
}

void ResultInfo::setIdentifier( QVariant identifier )
{
  m__Identifier = identifier;
}

const QString & ResultInfo::name() const
{
  return m__Name;
}

void ResultInfo::setName( const QString &name )
{
  m__Name = name;
}