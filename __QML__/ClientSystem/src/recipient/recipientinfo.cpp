#include "recipientinfo.h"

RecipientInfo::RecipientInfo() :
  QObject(NULL),
  m__Identifier(QVariant()),
  m__Name(QString())
{
}

RecipientInfo::~RecipientInfo()
{
}

QVariant RecipientInfo::identifier() const
{
  return m__Identifier;
}

void RecipientInfo::setIdentifier( QVariant identifier )
{
  m__Identifier = identifier;
}

QString RecipientInfo::name() const
{
  return m__Name;
}

void RecipientInfo::setName( QString name )
{
  m__Name = name;
}
