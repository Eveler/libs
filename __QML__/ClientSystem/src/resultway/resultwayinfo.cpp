#include "resultwayinfo.h"


ResultwayInfo::ResultwayInfo() :
  QObject(NULL),
  m__Identifier(QVariant()),
  m__Name(QString())
{
}

ResultwayInfo::~ResultwayInfo()
{
}

QVariant ResultwayInfo::identifier() const
{
  return m__Identifier;
}

void ResultwayInfo::setIdentifier( QVariant identifier )
{
  m__Identifier = identifier;
}

QString ResultwayInfo::name() const
{
  return m__Name;
}

void ResultwayInfo::setName( QString name )
{
  m__Name = name;
}
