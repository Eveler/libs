#ifndef MFCDOCUMENTZIPPER_H
#define MFCDOCUMENTZIPPER_H

#include "mfcdocumentioprovider.h"
#include <QObject>

#include "export/dossier_export.h"

class QSettings;
class DOSSIER_EXPORT MFCDocumentZipper : public QObject, public MFCDocumentIOProvider
{
    Q_OBJECT

  public:

    explicit MFCDocumentZipper( QObject *parent = NULL );

    void setFilePath( const QString &filePath );
    const QString & filePath() const;

    bool load( MFCDocument *doc );
    bool save(MFCDocument *doc, Format fmt = Version11);

    bool addPDF(MFCDocument *doc, QByteArray data,
                QSettings *requisites = NULL);

  signals:
    void dataTransferProgress( int curVal, int maxVal );


  private:
    QString m__FilePath;

    QString zipErrStr( int errn );
};

#endif // MFCDOCUMENTZIPPER_H
