#ifndef DIALOG_DOCDETAILS_H
#define DIALOG_DOCDETAILS_H

#include <QDialog>

#include "mfcdocumentinfo.h"


namespace Ui {
  class Dialog_DocDetails;
}

class Dialog_DocDetails : public QDialog
{
    Q_OBJECT


  public:
    enum WriteMode { WriteAll = 0, WriteRequisites, WritePagesnum };
    explicit Dialog_DocDetails(QWidget *parent = 0);
    ~Dialog_DocDetails();

    MFCDocumentInfo * exec( const QStringList &doctypes );
    int exec( MFCDocumentInfo *doc, WriteMode writeMode = WriteRequisites );


  private:
    Ui::Dialog_DocDetails *ui;
    MFCDocumentInfo *m__Document;

    void check( QString doc_type, QDate doc_date,
                bool isOriginals, int oExemplars, int oPages,
                bool isCopies, int cExemplars, int cPages );


  private slots:
    void on_cBox_Doctype_currentIndexChanged(int index);
    void on_dEdit_Date_dateChanged(const QDate &date);
    void on_gBox_Originals_toggled(bool arg1);
    void on_gBox_Copies_toggled(bool arg1);
};

#endif // DIALOG_DOCDETAILS_H
