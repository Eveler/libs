#ifndef ABSTRACTDOCLISTLOADER_H
#define ABSTRACTDOCLISTLOADER_H

#include <QObject>
#include <QSqlDatabase>
#include "documentsmodel.h"
#include "abstractdocsstorage.h"

class AbstractDocListLoader : public QObject
{
  Q_OBJECT

public:
  explicit AbstractDocListLoader(QSqlDatabase db,
                                 QObject *parent = 0);
  ~AbstractDocListLoader();
  void setStorage(AbstractDocsStorage *s);
  AbstractDocsStorage *storage() const;
  virtual DocumentsModel* load(QVariant foreignID)=0;
  bool load( MFCDocumentInfo *doc ) const;
  virtual void clear();

signals:
  void progress(qint64 val,qint64 total);
  void modelDestroyed();
  void error(QString);
  void done(bool);
  void documentLoadDone(MFCDocumentInfo*);

public slots:
  virtual void cancelDownload();

protected slots:
  void objectDestroyed();

protected:
  QSqlDatabase DB;
  DocumentsModel *docListModel;

  QString errStr;

  AbstractDocsStorage *docStorage;
  bool ownStorage;

  void setError(QString str);
  void beginAddDocuments();
  void endAddDocuments();
};

#endif // ABSTRACTDOCLISTLOADER_H
