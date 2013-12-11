#ifndef MQML_H
#define MQML_H

#include <QObject>

#include <QtQml>


class TreeItem;

class MQML : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( MQML )


  public:
    explicit MQML(QObject *parent = 0);

    Q_INVOKABLE TreeItem * createTreeItem( QVariant value = QVariant() ) const;

    Q_INVOKABLE int daysInMonth( QDate date );
    Q_INVOKABLE int daysInMonth( int year, int month );
};

QML_DECLARE_TYPE(MQML)

#endif // MQML_H