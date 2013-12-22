#ifndef CLIENTLOADER_H
#define CLIENTLOADER_H

#include <QObject>

#include <QtQml>


class ClientLoader_P;
class ClientInfo;

class ClientLoader : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ClientLoader)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString connectionName READ connectionName
               WRITE setConnectionName NOTIFY connectionNameChanged)
    Q_PROPERTY(bool started READ started NOTIFY startedChanged)


  public:
    ClientLoader(QObject *parent = 0);
    ~ClientLoader();

    Q_INVOKABLE QString lastError() const;

    QString connectionName();
    bool setConnectionName( QString connectionName );

    bool started() const;

    Q_INVOKABLE bool load( const QString &filter = QString(), bool blockUI = false );


  signals:
    void lastErrorChanged();
    void connectionNameChanged();
    void startedChanged();
    void newInfo( ClientInfo *info );


  public slots:


  private:
    ClientLoader_P *p;
    QEventLoop *loop;

  private slots:
    void threadStarted();
    void threadFinished();
    void receivedError( QString errorText );
};

QML_DECLARE_TYPE(ClientLoader)

#endif // CLIENTLOADER_H
