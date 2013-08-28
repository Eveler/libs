#ifndef MFCDOCUMENT_H
#define MFCDOCUMENT_H

#include <QObject>

#include <QUuid>
#include <QString>
#include <QDate>
#include <QByteArray>
#include <QTemporaryFile>
#include <QVariant>
#include "mfcdocumentpages.h"
#include "docattachments.h"
#include "export/dossier_export.h"

class DOSSIER_EXPORT MFCDocument : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString type READ type WRITE setType)
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(QString series READ series WRITE setSeries)
  Q_PROPERTY(QString number READ number WRITE setNumber)
  Q_PROPERTY(QDate date READ date WRITE setDate)
  Q_PROPERTY(QDate expires READ expiresDate WRITE setExpiresDate)
  Q_PROPERTY(QString agency READ agency WRITE setAgency)
  Q_PROPERTY(QDateTime created READ createDate WRITE setCreateDate)
  Q_PROPERTY(QString url READ url WRITE setUrl)

private:
  explicit MFCDocument( QObject *parent = 0 );
  ~MFCDocument();

public:
  static MFCDocument *instance(QString doc_type, QDate doc_date,
                               QDateTime doc_createdate, QObject *parent=0);

  bool copyFrom(MFCDocument *doc);
  void setType( const QString &doc_type );
  void setName( const QString &doc_name );
  void setSeries( const QString &doc_series );
  void setNumber( const QString &doc_number );
  void setDate( QDate doc_date );
  void setExpiresDate( QDate doc_expires );
  void setAgency( const QString &doc_agency );
  void setCreateDate( QDateTime doc_createdate );
  void setUrl(const QString docUrl);
  void addPage(MFCDocumentPage &page);
  bool replacePage(const int pageNum,MFCDocumentPage &newPage);
  void addAttachment(const QString fileName,const QString mimeType,
                     const QByteArray &fileData);

  const QString & type();
  const QString & name();
  const QString & series();
  const QString & number();
  const QDate & date();
  const QDate & expiresDate();
  const QString & agency();
  const QDateTime & createDate();
  const QString & url();
  bool havePages();
  MFCDocumentPages* pages();
  bool haveAttachments();
  DocAttachments* attachments();
  bool isChanged();
  bool isValid();
  int size();

  QString errorString();

  QUuid uuid();
  static MFCDocument *document( QUuid uuid );

signals:
  void type_Changed();
  void name_Changed();
  void series_Changed();
  void number_Changed();
  void date_Changed();
  void expiresDate_Changed();
  void agency_Changed();
  void createDate_Changed();
  void urlChanged();
  void propertyChanged(QString pName,QVariant val);
  void needBody(QString,MFCDocument*);
  void errorAccured(QString);

public slots:
  static void remove(MFCDocument *doc);

private slots:
  void remove();

private:
  static QHash< MFCDocument*,int > instances;
  static QHash< QUuid, MFCDocument * > doclist;
  QString m_Type;
  QString m_Name;
  QString m_Series;
  QString m_Number;
  QDate m_Date;
  QDate m_Expires;
  QString m_Agency;
  QDateTime m_CreateDate;
  QString m_url;
  bool changed;
  QString errStr;
  void init();
  void error(QString str);

protected:
  MFCDocumentPages *m_pages;
  DocAttachments *m_attachments;
};

#endif // MFCDOCUMENT_H
