#include "electrodoc_v2.h"
#include "ui_electrodoc_v2.h"
#include <QMessageBox>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsDropShadowEffect>
#include <QImageReader>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QTextDecoder>
#include <QMetaEnum>
#include <QPrintDialog>
#include <QUuid>
#include "mfcdocumentzipper.h"
#include "amslogger.h"
#include "mfccore.h"

ElectroDoc_v2::ElectroDoc_v2(QWidget *parent) :
  MFCWidget(parent),
  ui(new Ui::ElectroDoc_v2),
  #ifdef Q_OS_WIN
  ext_proc(NULL),
  #endif
  loopResult(-1),
  loop(new QEventLoop(this))
{
  ui->setupUi(this);
  canJustClose = true;

  ui->lstWgt_Attachments->clear();
  viewer=new DocPagesViewer(ui->wgt_Pages);
  connect(viewer,SIGNAL(pageBecomeVisible(int)),
          this,SLOT(setVisiblePage(int)));
  connect(viewer,SIGNAL(scaled(int)),this,SIGNAL(scaled(int)));
  connect(viewer,SIGNAL(progress(int,int)),ui->pBar_Scan,SLOT(setValue(int)));
  QVBoxLayout *layout=new QVBoxLayout(ui->wgt_Pages);
  layout->setSpacing(0);
  layout->addWidget(viewer);
  ui->wgt_Pages->setLayout(layout);

  ui->pBar_Scan->setVisible( false );
  ui->wgt_ExtDoc->setVisible(false);

  ui->dEdit_DocDate->setMinimumDate(QDate(1800,1,1));
  ui->dEdit_DocDate->setDate(QDate(1800,1,1));
  ui->dEdit_DocExpires->setMinimumDate(QDate(1800,1,1));
  ui->dEdit_DocExpires->setDate(ui->dEdit_DocExpires->minimumDate());

  previousScaleValue = ui->hSlider_Scale->value();
  ui->hSlider_Scale->setValue( previousScaleValue );
  previousScaleValue *= 10;

  setReadOnly(false);
  setRestrictQuality();
  fWatcher=new QFileSystemWatcher(this);
  connect(fWatcher,SIGNAL(fileChanged(QString)),this,SLOT(fileChanged(QString)));

  m_Document=NULL;
  originalDocument=NULL;
  title=tr("Электронный документ");
  setModified(false);
  saved=false;
//#ifdef Q_OS_WIN
//  m_pTwain = NULL;
//#endif

  ui->cBox_DocType->setFocus();
}

ElectroDoc_v2::~ElectroDoc_v2()
{
#ifdef Q_OS_WIN
  if(ext_proc){
    ext_proc->disconnect();
    ext_proc->terminate();
    if(!ext_proc->waitForFinished(3000)) ext_proc->kill();
    delete ext_proc;
  }
#endif

  clear();
  delete viewer;
  delete fWatcher;
//#ifdef Q_OS_WIN
//  releaseTWAIN();
//#endif
  delete ui;
}

bool ElectroDoc_v2::setDetails(const Details details, const QVariant val)
{
  switch(details){
    case Type:
      ui->cBox_DocType->clear();
      if ( val.type() == QVariant::String )
        ui->cBox_DocType->addItem( val.toString() );
      else if ( val.type() == QVariant::StringList )
      {
        ui->cBox_DocType->addItems( val.toStringList() );
        if ( ui->cBox_DocType->count() > 1 )
          ui->cBox_DocType->setCurrentIndex( -1 );
      }
      break;
    case Name:
      ui->lEdit_DocName->setText(val.toString());
      break;
    case Series:
      ui->lEdit_DocSer->setText(val.toString());
      break;
    case Number:
      ui->lEdit_DocNum->setText(val.toString());
      break;
    case Date:
      ui->dEdit_DocDate->setDate(val.toDate());
      break;
    case ExpiresDate:
      ui->dEdit_DocExpires->setDate(val.toDate());
      break;
    case Agency:
      ui->cBox_DocAgency->clear();
      if ( val.type() == QVariant::String )
        ui->cBox_DocAgency->addItem( val.toString() );
      else if ( val.type() == QVariant::StringList )
      {
        ui->cBox_DocAgency->addItems( val.toStringList() );
        if ( ui->cBox_DocAgency->count() > 1 )
          ui->cBox_DocAgency->setCurrentIndex( -1 );
      }
      break;
    case CreateDate:
      if(!m_Document) return false;
      m_Document->setCreateDate(val.toDateTime());
      break;
  }
  return true;
}

void ElectroDoc_v2::setState( State state )
{
  m__State=state;
  switch (state)
  {
    case Create:
      canJustClose = true;
      setReadOnly( false );
      ui->tBt_SaveDocument->setVisible( true );
      ui->tBt_RejectDocument->setVisible( true );
      ui->tBt_RejectDocument->setText( tr( "Отмена" ) );
      break;
    case Check:
      canJustClose = true;
      setReadOnly( true );
      ui->tBt_SaveDocument->setVisible( true );
      ui->tBt_RejectDocument->setVisible( true );
      ui->tBt_RejectDocument->setText( tr( "Отмена" ) );
      break;
    case Read:
      canJustClose = true;
      setReadOnly( true );
      ui->tBt_SaveDocument->setVisible( false );
      ui->tBt_RejectDocument->setVisible( true );
      ui->tBt_RejectDocument->setText( tr( "Закрыть" ) );
      break;
    default:
      canJustClose = true;
      setReadOnly( false );
      ui->wgt_Details->setEnabled( false );
      ui->tBt_RemoveExt->setEnabled( false );
      ui->tBt_LoadExt->setEnabled( true );
      ui->tBt_DeletePage->setEnabled(state==CopyCreate);
      ui->tBt_SaveDocument->setVisible( true );
      ui->tBt_RejectDocument->setVisible( true );
      ui->tBt_RejectDocument->setText( tr( "Отмена" ) );
      break;
  }
}

bool ElectroDoc_v2::setDocument(MFCDocument *document){
  if(document==NULL || !(document->haveAttachments() || document->havePages()))
    return false;
  // Очищаем виджеты
  clear();
//  m_Document=document;
  originalDocument=document;
  if(m_Document==NULL)
    m_Document=MFCDocument::instance();
  m_Document->copyFrom(document);
//  m_Document->setProperty("created_in",tr("%1 (%2)").arg(__FILE__).arg(__LINE__));

  ui->pBar_Scan->setFormat( tr("Загрузка: %p%") );

  if(m_Document->haveAttachments()){
    ui->pBar_Scan->setMaximum(m_Document->attachments()->count());
    ui->pBar_Scan->setMinimum(0);
    ui->pBar_Scan->setValue(0);
    ui->pBar_Scan->setVisible(true);
    ui->wgt_ExtDoc->setVisible(true);
    for(int a=0;a<m_Document->attachments()->count();a++){
      if(!m_Document->attachments()->getAttachment(a).fileName().isEmpty())
        ui->lstWgt_Attachments->addItem(
              m_Document->attachments()->getAttachment(a).fileName());
      ui->pBar_Scan->setValue(a+1);
    }
    ui->pBar_Scan->setVisible(false);
  }else ui->wgt_ExtDoc->setVisible(false);

  if(m_Document->havePages()){
    MFCDocumentPages *docPages=m_Document->pages();
    if(docPages==NULL) return false;
    ui->pBar_Scan->setMaximum(docPages->count());
    ui->pBar_Scan->setMinimum(0);
    ui->pBar_Scan->setValue(0);
    ui->pBar_Scan->setVisible(true);
    for(int p=0;p<docPages->count();p++){
      MFCDocumentPage* pg=docPages->getPage(p);
      ui->lWgt_Pages->addItem(pg->getPageName());
      if(ui->lWgt_Pages->count()==1){
        ui->lWgt_Pages->setCurrentRow(0);
      }
      ui->pBar_Scan->setValue(p+1);
      qApp->processEvents();
    }
    viewer->documentChanged(m_Document);
  }

//  ui->cBox_DocType->setCurrentIndex(
//        ui->cBox_DocType->findText(m_Document->type()));
  setDetails( Type, m_Document->type() );
  setDetails( Name, m_Document->name() );
  setDetails( Series, m_Document->series() );
  setDetails( Number, m_Document->number() );
  setDetails( Date, m_Document->date() );
  setDetails( ExpiresDate, m_Document->expiresDate() );
  setDetails( Agency, m_Document->agency() );

  ui->pBar_Scan->setVisible(false);
  setModified(false);
  return true;
}

MFCDocument *ElectroDoc_v2::document() const{
  return originalDocument;
}

QString ElectroDoc_v2::errorString(){
  return errStr;
}

QString ElectroDoc_v2::transliter(const QString &str){
  // (C) linux.org.ru wingear
  QHash<QString, QString> t;

  t.insert("а","a");  t.insert("б","b");  t.insert("в","v");   t.insert("г","g");
  t.insert("д","d");  t.insert("е","ye"); t.insert("ё","yo");  t.insert("ж","zh");
  t.insert("з","z");  t.insert("и","i");  t.insert("й","y");   t.insert("к","k");
  t.insert("л","l");  t.insert("м","m");  t.insert("н","n");   t.insert("о","o");
  t.insert("п","p");  t.insert("р","r");  t.insert("с","s");   t.insert("т","t");
  t.insert("у","u");  t.insert("ф","f");  t.insert("х","kh");  t.insert("ц","c");
  t.insert("ч","ch"); t.insert("ш","sh"); t.insert("щ","sch"); t.insert("ъ","");
  t.insert("ы","y");  t.insert("ь","");   t.insert("э","e");   t.insert("ю","yu");
  t.insert("я","ya");
  t.insert("А","A");  t.insert("Б","B");  t.insert("В","V");   t.insert("Г","G");
  t.insert("Д","D");  t.insert("Е","YE"); t.insert("Ё","YO");  t.insert("Ж","ZH");
  t.insert("З","Z");  t.insert("И","I");  t.insert("Й","Y");   t.insert("К","K");
  t.insert("Л","L");  t.insert("М","M");  t.insert("Н","N");   t.insert("О","O");
  t.insert("П","P");  t.insert("Р","R");  t.insert("С","S");   t.insert("Т","T");
  t.insert("У","U");  t.insert("Ф","F");  t.insert("Х","KH");  t.insert("Ц","C");
  t.insert("Ч","CH"); t.insert("Ш","SH"); t.insert("Щ","SCH"); t.insert("Ъ","");
  t.insert("Ы","Y");  t.insert("Ь","");   t.insert("Э","E");   t.insert("Ю","YU");
  t.insert("Я","YA");
  t.insert("№","N");
//  t[0x430] = "a";
//  t[0x410] = "A";
//  t[0x44f] = "ya";
//  t[0x42f] = "Ya";

  QString result;

  foreach(QChar ch,str){
    result+=(t.contains(ch))?t[ch]:ch;
  }

  return result;
}

void ElectroDoc_v2::setRestrictQuality(const bool r){
  restrictQuality=r;
}

void ElectroDoc_v2::clear(){
  ui->lEdit_DocName->clear();
  ui->lEdit_DocNum->clear();
  ui->lEdit_DocSer->clear();
  ui->cBox_DocAgency->setCurrentIndex(-1);
  ui->cBox_DocType->setCurrentIndex(-1);
  ui->dEdit_DocDate->setDate(QDate());
  ui->dEdit_DocExpires->setDate(QDate());
  ui->lWgt_Pages->clear();
  ui->lstWgt_Attachments->clear();
  viewer->clear();
  if(m_Document!=NULL){
    MFCDocument::remove(m_Document);
    m_Document=NULL;
  }
  setModified(false);
}

bool ElectroDoc_v2::isModified(){
  return m_modified;
}

void ElectroDoc_v2::setModified(const bool m){
  if(!m && isReadOnly){
    m_modified=false;
    return;
  }
  m_modified=m;
  setWindowTitle(title+(m?" *":""));
}

QString ElectroDoc_v2::detailsName(const Details details) const{
  QMetaEnum en=metaObject()->enumerator(metaObject()->indexOfEnumerator("Details"));
  return tr(en.valueToKey(details));
}

int ElectroDoc_v2::exec( bool maximized )
{
  if ( loop->isRunning() ) return -1;
  loopResult = -1;
  if ( maximized ) showMaximized();
  else show();
  return loop->exec();
}

void ElectroDoc_v2::closeEvent(QCloseEvent *e){
  if(isModified() && ui->tBt_SaveDocument->isEnabled() && canJustClose){
    int answer=QMessageBox::question(this,tr("Внимание"),
                                     tr("Документ изменён! Выйти без сохранения"),
                                     tr("Да"),tr("Нет"));
    if( answer < 1 )
    {
      e->accept();
      emit closed();
      loop->exit( loopResult );
    }
    else
    {
      e->ignore();
      setVisible(true);
      return;
    }
  }
  else if( saved && canJustClose )
  {
    emit saveCompleted(true,originalDocument/*m_Document*/);
    emit closed();
    loop->exit( loopResult );
  }
  else if(!canJustClose){
    e->ignore();
    setVisible(true);
    return;
  }
  else
  {
    loop->exit( loopResult );
    emit closed();
  }
  QWidget::closeEvent(e);
}

//#ifdef Q_OS_WIN
//bool ElectroDoc_v2::nativeEvent( const QByteArray &eventType, void *message, long *result )
//{
//  Q_UNUSED(eventType);
//  return winEvent( (MSG*)message, result );
//}

//bool ElectroDoc_v2::winEvent( MSG *message, long */*result*/ )
//{
//  if ( m_pTwain != NULL )
//    m_pTwain->processMessage(*message);
//  return false;
//}
//#endif

void ElectroDoc_v2::setReadOnly(const bool readOnly){
  isReadOnly=readOnly;
  ui->wgt_ToolBar->setEnabled( !isReadOnly );
  ui->wgt_PageEdit->setEnabled( !isReadOnly );
  ui->wgt_Details->setEnabled( !isReadOnly );
  ui->tBt_RemoveExt->setEnabled( !isReadOnly );
  ui->tBt_LoadExt->setEnabled( !isReadOnly );
  disconnect(ui->tBt_SaveDocument,SIGNAL(clicked()),this,SLOT(confirm()));
  disconnect(ui->tBt_SaveDocument,SIGNAL(clicked()),this,SLOT(save()));
  if(isReadOnly){
    connect(ui->tBt_SaveDocument,SIGNAL(clicked()),this,SLOT(confirm()));
    ui->tBt_SaveDocument->setText(tr("Да, документ верный"));
  }
  else
  {
    connect(ui->tBt_SaveDocument,SIGNAL(clicked()),this,SLOT(save()));
    ui->tBt_SaveDocument->setText(tr("Сохранить документ"));
  }
}

void ElectroDoc_v2::addPage(const QString &pName, const QPixmap &pixmap){
  if(pixmap.isNull()){
    setError(tr("Ошибка при добавлении изображения"));
    return;
  }
  if(m_Document==NULL){
    m_Document=MFCDocument::instance();
//    m_Document->setProperty("created_in",tr("%1 (%2)").arg(__FILE__).arg(__LINE__));
  }
//  pixmap.d
  MFCDocumentPage *pg=new MFCDocumentPage(pName,pixmap);
  m_Document->addPage(*pg);

  ui->lWgt_Pages->addItem(pName);
  if ( ui->lWgt_Pages->count() == 1 )
  {
    ui->lWgt_Pages->setCurrentRow( 0 );
  }
  setModified(true);
  viewer->documentChanged(m_Document);
}

void ElectroDoc_v2::addAttachment(const QString fileName,
                                  const QString mimeType,
                                  const QByteArray &fileData){
  if(fileName.isEmpty()) return;
  if(m_Document==NULL){
    m_Document=MFCDocument::instance();
//    m_Document->setProperty("created_in",tr("%1 (%2)").arg(__FILE__).arg(__LINE__));
  }

  if(ui->lstWgt_Attachments->findItems(fileName,Qt::MatchExactly).count()==0){
    m_Document->addAttachment(fileName,mimeType,fileData);
    ui->lstWgt_Attachments->addItem(fileName);
  }else{
    setError(tr("Вложение %1 уже есть").arg(fileName));
  }
  setModified(true);
}

void ElectroDoc_v2::removeSelectedAttachments(){
  if(!m_Document) return;
  if(ui->lstWgt_Attachments->selectedItems().count()==0) return;
  QList< QListWidgetItem* > items=ui->lstWgt_Attachments->selectedItems();
  foreach(QListWidgetItem* item,items){
    QString fileName=item->text();
    m_Document->attachments()->removeAttachment(fileName);
    delete ui->lstWgt_Attachments->takeItem(ui->lstWgt_Attachments->row(item));
  }
  setModified(true);
}

bool ElectroDoc_v2::replacePage(const int pageNum, const QPixmap &pixmap){
  if(!m_Document || pageNum<0 || pixmap.isNull()){
    setError(tr("Ошибка при замене изображения"));
    return false;
  }
  MFCDocumentPage *pg=new MFCDocumentPage(
        m_Document->pages()->getPage(pageNum)->getPageName(),pixmap);
  if(!m_Document->replacePage(pageNum,*pg)){
    setError(tr("Ошибка при замене изображения: %1").arg(
               m_Document->errorString()));
    return false;
  }
  setModified(true);
  viewer->documentChanged(m_Document);
  return true;
}

void ElectroDoc_v2::loadExtFile(const QString fName){
  QString mimeType=QString();
  QFileInfo fi(fName);
#ifdef Q_OS_LINUX
  QProcess p(this);
  p.setProcessChannelMode(QProcess::MergedChannels);
  p.start("xdg-mime query filetype \""+fi.absoluteFilePath()+"\"");
  p.waitForStarted();
  p.waitForFinished();
  QByteArray output=p.readAllStandardOutput().simplified();
  qDebug()<<output;
  if(output.length()>0 && output.contains("/")) mimeType=output;
#endif
  if(mimeType.length()==0){
    if(fi.suffix().toLower()=="pdf")
      mimeType="application/pdf";
    else if(fi.suffix().toLower()=="rtf")
      mimeType="application/rtf";
    else if(fi.suffix().toLower()=="txt")
      mimeType="text/plain";
    else if(fi.suffix().toLower()=="gz")
      mimeType="application/x-gzip";
    else if(fi.suffix().toLower()=="zip")
      mimeType="application/zip";
    else if(fi.suffix().toLower()=="bz2")
      mimeType="application/x-bzip2";
    else if(fi.suffix().toLower()=="htm" || fi.suffix().toLower()=="html")
      mimeType="text/html";
    else if(fi.suffix().toLower()=="doc" || fi.suffix().toLower()=="docx")
      mimeType="application/msword";
    else if(fi.suffix().toLower()=="pps" || fi.suffix().toLower()=="ppsx")
      mimeType="application/mspowerpoint";
    else if(fi.suffix().toLower()=="xls" || fi.suffix().toLower()=="xlsx")
      mimeType="application/x-excel";
  }
  QFile f(fi.absoluteFilePath());
  f.open(QFile::ReadOnly);
  QByteArray fileData=f.readAll();
  f.close();
  QString fileName=transliter(fi.fileName());
  if(fileData.length()>0){
    fileName.replace("`","_");
    fileName.replace("!","_");
    fileName.replace("@","_");
    fileName.replace("$","_");
    fileName.replace(";","_");
    fileName.replace("%","_");
    fileName.replace("^","_");
    fileName.replace(":","_");
    fileName.replace("&","_");
    fileName.replace("?","_");
    fileName.replace("*","_");
    fileName.replace("+","_");
    fileName.replace("=","_");
    fileName.replace("|","_");
    fileName.replace("/","_");
    fileName.replace("\"","_");
    fileName.replace("'","_");
    fileName.replace(",","_");
    fileName.replace("<","_");
    fileName.replace(">","_");
    fileName.replace(" ","_");
    fileName.replace("(","_");
    fileName.replace(")","_");
    addAttachment(fileName,mimeType,fileData);
  }
}

//#ifdef Q_OS_WIN
//void ElectroDoc_v2::initTWAIN()
//{
//    m_pTwain = new QTwain( NULL );
//    m_pTwain->setParent( this );
//    m_pTwain->setEmitPixmaps();
//    QObject::connect( m_pTwain, SIGNAL(pixmapAcquired(QPixmap*)),
//                      SLOT(pixmapAcquired(QPixmap*)) );
//}

//void ElectroDoc_v2::releaseTWAIN()
//{
//    if ( m_pTwain != NULL )
//    {
//        delete m_pTwain;
//        m_pTwain = NULL;
//    }
//}
//#endif

void ElectroDoc_v2::showProgress(qint64 done, qint64 total){
  ui->pBar_Scan->setMaximum(total);
  ui->pBar_Scan->setValue(done);
}

void ElectroDoc_v2::scale(int value){
  viewer->scale(value);
}

void ElectroDoc_v2::fitToWith(){
  viewer->scaleToViewWith();
}

void ElectroDoc_v2::scaleToOriginal(){
  ui->hSlider_Scale->setValue(100);
}

void ElectroDoc_v2::zoomIn(){
  viewer->scale(ui->hSlider_Scale->value()+10);
}

void ElectroDoc_v2::zoomOut(){
  viewer->scale(ui->hSlider_Scale->value()-10);
}

#ifdef Q_OS_WIN
void ElectroDoc_v2::execTwainProc(const QString &param)
{
  ext_proc = new QProcess(this);
  ext_proc->setProcessChannelMode(QProcess::MergedChannels);
  connect(ext_proc,SIGNAL(finished(int)),SLOT(processFinished(int)));
  connect(ext_proc,SIGNAL(error(QProcess::ProcessError)),
          SLOT(processError(QProcess::ProcessError)));
//  LogDebug()<<tr("TwainGui.exe %1").arg(param);
  ext_proc->start(tr("TwainGui.exe %1").arg(param));

  if(ext_proc && !ext_proc->waitForStarted()){
    QMessageBox::warning(this, tr("Ошибка"),
                         tr("Истекло время ожидания запуска процесса: %1")
                         .arg(ext_proc?ext_proc->errorString():""));
    if(ext_proc){
      ext_proc->disconnect();
      delete ext_proc;
      ext_proc=NULL;
    }
  }
}

void ElectroDoc_v2::processFinished(int exitCode)
{
  if(!ext_proc) return;

  // загрузим все файлы temp/*.png ********************************************/
  QDir d("temp");
  QStringList entryList = d.entryList(
        QStringList()<<"*.png"<<"*.PNG",
        QDir::Files|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System,
        QDir::Name);
  ui->pBar_Scan->setFormat( "Загрузка: %p%" );
  ui->pBar_Scan->setVisible( true );
  ui->pBar_Scan->setMaximum(entryList.count()-1);
  int processed = 0;
  foreach(QString file, entryList){
    QString fName = "temp/"+file;
//    LogDebug()<<"loading"<<fName;
    loadImage(fName);
    QFile::remove(fName);
//    LogDebug()<<fName<<"DONE";
    ui->pBar_Scan->setValue(processed++);
    qApp->processEvents();
//    LogDebug()<<"END foreach iteration";
  }
  ui->pBar_Scan->hide();
  //******************************************* загрузим все файлы temp/*.png //

  QString errStr;
  if(exitCode==-2){
    errStr=tr("Нет возможности запустить дочерний процесс: %1")
             .arg(ext_proc->errorString());
  }else if(exitCode==-1){
    errStr=tr("Дочерний процесс внезапно завершился: %1")
             .arg(ext_proc->errorString());
  }else if(!ext_proc->errorString().isEmpty() &&
           ext_proc->errorString()!="Unknown error")
    errStr=tr("Дочерний процесс вернул ошибку: %1").arg(ext_proc->errorString());
  QString sys_out=ext_proc->readAllStandardOutput()+" "+
      ext_proc->readAllStandardError();
  if(sys_out.simplified().length()>0)
    errStr+=tr("Вывод процесса: %1").arg(sys_out);

  if(errStr.length()>0) QMessageBox::warning(this, tr("Внимание"), errStr);

  ext_proc->disconnect();
  delete ext_proc;
  ext_proc=NULL;

  ui->tBt_SelectSource->setEnabled(true);
  ui->tBt_ScanNew->setEnabled(true);
  ui->tBt_SaveDocument->setEnabled(true);
}

void ElectroDoc_v2::processError(QProcess::ProcessError err)
{
  if(err==QProcess::FailedToStart || err==QProcess::Crashed ||
     err==QProcess::Timedout || err==QProcess::UnknownError){
//    LogDebug()<<"Error while TwainGui works:"<<err;
    if(ext_proc){
//      LogDebug()<<ext_proc->errorString();
      QMessageBox::warning(this, tr("Ошибка"), ext_proc->errorString());
      ext_proc->disconnect();
      delete ext_proc;
      ext_proc=NULL;
    }
  }

  ui->tBt_SelectSource->setEnabled(true);
  ui->tBt_ScanNew->setEnabled(true);
  ui->tBt_SaveDocument->setEnabled(true);
}

void ElectroDoc_v2::scannerConfigTriggered(/*pos*/){
  ui->tBt_SelectSource->setDisabled(true);
//    releaseTWAIN();
//    initTWAIN();
//    m_pTwain->selectSource();
  ui->tBt_SaveDocument->setDisabled(true);
  execTwainProc("-c");

//    ui->tBt_SelectSource->setEnabled(true);
}

void ElectroDoc_v2::scannerStart()
{
  ui->tBt_ScanNew->setDisabled(true);
//    releaseTWAIN();
//    initTWAIN();
//    m_pTwain->acquire();
  ui->tBt_SaveDocument->setDisabled(true);
  execTwainProc("-s");

//    ui->tBt_ScanNew->setEnabled(true);
}

void ElectroDoc_v2::pixmapAcquired( QPixmap *pix )
{
  ui->tBt_ScanNew->setEnabled(true);
    if ( pix == NULL ) return;

    addPage( tr("Страница %1 (Скан %2)").arg(ui->lWgt_Pages->count()+1).arg(
                 ui->lWgt_Pages->count()+1), *pix );
    delete pix;
    pix = NULL;
}
#endif

void ElectroDoc_v2::loadImage(){
  QStringList supportedFormats;
  foreach ( QString formatName, QImageReader::supportedImageFormats() )
    supportedFormats << formatName;
  supportedFormats << "pdf";
  QStringList fNames=QFileDialog::getOpenFileNames(
        this,tr("Выберите файл(ы) изображений"),"",
                       QString( "Image files (*.%1 *.%2)" )
                       .arg( supportedFormats.join( " *." ),
                             supportedFormats.join( " *." ).toUpper() ) );
  if(fNames.count()==0) return;
  ui->pBar_Scan->setFormat( "Загрузка: %p%" );
  ui->pBar_Scan->setVisible( true );
  ui->pBar_Scan->setMaximum(fNames.count()-1);
  for( int fIdx = 0; fIdx < fNames.count(); fIdx++ )
  {
    ui->pBar_Scan->setValue(fIdx);
    qApp->processEvents();
    QString fName = fNames.at( fIdx );
    loadImage(fName);
  }
  ui->pBar_Scan->setVisible(false);

  viewer->documentChanged(m_Document);
}

void ElectroDoc_v2::loadImage(const QString &fName)
{
  QFileInfo fi(fName);
  if(fi.suffix().toLower().contains("pdf")){
    MFCDocumentZipper provider;
    connect(&provider, SIGNAL(dataTransferProgress(int,int)), ui->pBar_Scan,
            SLOT(setValue(int)));
    if(m_Document==NULL) m_Document=MFCDocument::instance();
    QFile pdf(fName);
    if(!pdf.open(QFile::ReadOnly)){
      QMessageBox::warning(this, tr("Ошибка"),
                           tr("Ошибка открытия файла: %1").arg(pdf.errorString()));
      return;
    }
    if(!provider.addPDF(m_Document, pdf.readAll())){
      QMessageBox::warning(this, tr("Ошибка"),
                           tr("Ошибка чтения файла: %1").arg(provider.error()));
    }
    pdf.close();

    ui->lWgt_Pages->clear();
    MFCDocumentPages *docPages = m_Document->pages();
    for(int p=0;p<docPages->count();p++){
      MFCDocumentPage* pg=docPages->getPage(p);
      ui->lWgt_Pages->addItem(pg->getPageName());
      if ( ui->lWgt_Pages->count() == 1 )
      {
        ui->lWgt_Pages->setCurrentRow( 0 );
      }
    }
    setModified(true);
    viewer->documentChanged(m_Document);
    return;
  }else{
    QPixmap img;
    if(img.load(fName)){
      addPage(tr("Страница %1 (Скан %2)").arg(ui->lWgt_Pages->count()+1).arg(
                ui->lWgt_Pages->count()+1),
              img);
    }
    else
    {
      QMessageBox::warning( NULL, tr( "Ошибка" ),
                            tr( "Произошла неизвестная ошибка при попытке "
                                "прочитать файл изображения.\nФайл: %1" )
                            .arg( fName ) );
    }
  }
}

void ElectroDoc_v2::loadAttachment(){
  QString fName=QFileDialog::getOpenFileName(
        this,tr("Выберите файл"),QString(),
        tr("Предпочтительные типы документов (*.odt *.txt *.ods *.pdf *.doc *.docx *.xls *.xlsx "
           "*.html *.htm *.rtf *.csv *.pps *.ppsx);;Все файлы (*.*);;"
           "Текст Open Document (*.odt);;"
           "Текстовые файлы (*.txt);;Таблица Open Document (*.ods);;"
           "Portable Document Format (*.pdf);;Comma separated files (*.csv)"
           "Rich Text Format (*.rtf);;HTML files (*.html *.htm);;"
           "Документ MS Word (*.doc *.docx);;Таблица MS Excel (*.xls *.xlsx);;"
           "Документ MS Powerpoint (*.pps *.ppsx);;"
           "Архивы (*.7z *.rar *.zip *.bz2 *.gz)"));
  if(fName.length()==0) return;
  ui->wgt_ExtDoc->setVisible(true);
  loadExtFile(fName);
}

void ElectroDoc_v2::setVisiblePage(int pageNum){
  viewer->setVisiblePage(pageNum);

  ui->lWgt_Pages->setCurrentRow(pageNum);
  int count=ui->lWgt_Pages->count();
  ui->tBt_Print->setEnabled(count>0);
  ui->gBox_Pages->setTitle(
        tr("Документ - стр. %1 из %2").arg(count==0?0:pageNum+1).arg(count));

  ui->tBt_PageUp->setDisabled(ui->lWgt_Pages->currentRow()<=0);
  ui->tBt_PageDown->setDisabled(ui->lWgt_Pages->currentRow()>=
                                ui->lWgt_Pages->count()-1);
}

void ElectroDoc_v2::movePage(const int from, const int to){
  if(!m_Document || from<0 || to<0 || from==to
     || from>m_Document->pages()->count() || to>m_Document->pages()->count())
    return;
  MFCDocumentPage *page1=new MFCDocumentPage(
        m_Document->pages()->getPage(from)->getPageName(),
        m_Document->pages()->getPage(from)->getBody());
  MFCDocumentPage *page2=new MFCDocumentPage(
        m_Document->pages()->getPage(to)->getPageName(),
        m_Document->pages()->getPage(to)->getBody());
  if(!page1->isValid() || !page2->isValid()){
    setError(tr("Ошибка при создании промежуточных данных"));
    delete page1;
    delete page2;
    return;
  }
  if(!m_Document->replacePage(from,*page2)){
    QMessageBox::warning(this,tr("Ошибка"),m_Document->errorString());
    return;
  }
  if(!m_Document->replacePage(to,*page1)){
    QMessageBox::warning(this,tr("Ошибка"),m_Document->errorString());
    return;
  }
  setModified(true);
  viewer->documentChanged(m_Document);
}

void ElectroDoc_v2::moveCurrentPageUp(){
  movePage(ui->lWgt_Pages->currentRow(),ui->lWgt_Pages->currentRow()-1);
}

void ElectroDoc_v2::moveCurrentPageDown(){
  movePage(ui->lWgt_Pages->currentRow(),ui->lWgt_Pages->currentRow()+1);
}

void ElectroDoc_v2::removeCurrentPage(){
  if(!m_Document) return;
  int pageNum=ui->lWgt_Pages->currentRow();
  if(pageNum<0) return;
  if(!m_Document->pages()->removePage(pageNum)){
     QMessageBox::warning(this,tr("Ошибка"),m_Document->errorString());
     return;
  }
  QListWidgetItem *item=ui->lWgt_Pages->takeItem(pageNum);
  if(item!=NULL) delete item;
  ui->lWgt_Pages->setCurrentRow(pageNum);
  setModified(true);
  viewer->documentChanged(m_Document);
}

void ElectroDoc_v2::rotate(const int angle){
  if(!m_Document) return;
  QPixmap pix=QPixmap();
  int pageNum=ui->lWgt_Pages->currentRow();
  if(pageNum<0) return;
  pix.loadFromData(m_Document->pages()->getPage(pageNum)->getBody());
  if(pix.isNull()) return;
  QTransform transform;
  pix=pix.transformed(transform.rotate(angle),Qt::SmoothTransformation);
  replacePage(pageNum,pix);
}

void ElectroDoc_v2::rotateLeft(){
  rotate(-90);
}

void ElectroDoc_v2::rotateRight(){
  rotate(90);
}

void ElectroDoc_v2::saveToFile(){
  if(m_Document==NULL ||
     !(m_Document->haveAttachments() || m_Document->havePages()))
    return;
  QString fTypes=tr("Архив ZIP (*.zip)");
  if(m_Document->havePages()) fTypes+=tr(";;Файл формата PDF (*.pdf);;"
      "Набор изображений JPEG (*.jpg);;"
      "Набор изображений PNG (*.png)");
  QString fName=QFileDialog::getSaveFileName(
        this,tr("Сохранить в..."),"",fTypes);
  if(fName.isEmpty()) return;
  QFileInfo fi(fName);
  if(fi.suffix().toLower()==tr("zip")){
    ui->pBar_Scan->setFormat(tr("Обработка: %p%"));
    ui->pBar_Scan->setMinimum(0);
    ui->pBar_Scan->setVisible(true);
    qApp->processEvents();
    MFCDocumentZipper zipper;
    zipper.setFilePath( fi.absoluteFilePath() );
    connect( &zipper, SIGNAL(dataTransferProgress(int,int)),
             ui->pBar_Scan, SLOT(setRange(int,int)) );
    zipper.save( m_Document );
    ui->pBar_Scan->setVisible(false);
  }else if(fi.suffix().toLower()=="png" || fi.suffix().toLower()=="jpg" ||
           fi.suffix().toLower()=="jpeg"){
    // создаём набор файлов "baseName+<page #>.<ext>"
    ui->pBar_Scan->setVisible( true );
    ui->pBar_Scan->setMaximum(m_Document->pages()->count());
    ui->pBar_Scan->setFormat( "Сохранение: %p%" );
    for(int pIdx = 0; pIdx < m_Document->pages()->count(); pIdx++ )
    {
      QPixmap pixmap=QPixmap();
      MFCDocumentPage *page=m_Document->pages()->getPage(pIdx);
      pixmap.loadFromData(page->getBody());
      pixmap.save(
            fi.absolutePath()+"/"+fi.completeBaseName()+
            QVariant(pIdx+1).toString()+"."+fi.suffix(),
            fi.suffix().toLower()=="png"?"PNG":"JPG");
      ui->pBar_Scan->setValue(pIdx*100/m_Document->pages()->count());
    }
    ui->pBar_Scan->setVisible( false );
    if(m_Document->haveAttachments()){
      for(int pIdx = 0; pIdx < m_Document->attachments()->count(); pIdx++ ){
        const DocAttachment &att=m_Document->attachments()->getAttachment(pIdx);
        QFile f(fi.absolutePath()+"/"+att.fileName());
        f.open(QFile::WriteOnly);
        f.write(att.data());
        f.close();
      }
    }
  }else if(fi.suffix().toLower()=="pdf"){
    QPrinter printer;
    printer.setOutputFileName(fi.absoluteFilePath());
    printer.setPaperSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Portrait);
    printer.setPageMargins(5, 5, 5, 5, QPrinter::Millimeter);

    ui->pBar_Scan->setVisible( true );
    MFCDocumentPages *pages = m_Document->pages();
    int count = pages->count();
    ui->pBar_Scan->setMaximum(count);
    ui->pBar_Scan->setFormat( "Сохранение: %p%" );

    QPainter painter;
    painter.begin(&printer);

    for(int pIdx=0;pIdx<count;pIdx++){
      QPixmap pixmap = QPixmap();
      MFCDocumentPage *page = pages->getPage(pIdx);
      pixmap.loadFromData(page->getBody());
      painter.drawPixmap(
            0, 0, pixmap.scaled(printer.pageRect().size(),
                                Qt::KeepAspectRatio, Qt::SmoothTransformation));
      if(pIdx<count-1) printer.newPage();
      ui->pBar_Scan->setValue(pIdx*100/count);
    }

    painter.end();
    ui->pBar_Scan->setVisible( false );
  }
}

void ElectroDoc_v2::print(){
  if(!m_Document || !m_Document->havePages()) return;
//  QPrinter *printer=new QPrinter;

  QString pdf = QUuid::createUuid().toString().mid(1);
  pdf.chop(1);
  pdf = tr("print%1.pdf").arg(pdf);

  QPrinter printer;
  printer.setPaperSize(QPrinter::A4);
  printer.setOrientation(QPrinter::Portrait);
  printer.setPageMargins(5, 5, 5, 5, QPrinter::Millimeter);

  printer.setOutputFileName(pdf);
  printer.setOutputFormat(QPrinter::PdfFormat);

//  QPrintPreviewDialog *printPreview=new QPrintPreviewDialog( printer );
  //  QPrintDialog *printDialog = new QPrintDialog(&printer, this);

//  ui->pBar_Scan->setMaximum(m_Document->pages()->count());
//  ui->pBar_Scan->setFormat(tr("Печать: %p%"));
//  ui->pBar_Scan->setVisible(true);
//  qApp->processEvents();

//  printer->setPageSize(QPrinter::A4);
//  printer->setPaperSize(QPrinter::A4);
//  printer->setOrientation(QPrinter::Portrait);
//  printer->setPageMargins(5,5,5,5,QPrinter::Millimeter);
//  printPreview->setWindowState(Qt::WindowMaximized);

//  QObject::connect( printPreview, SIGNAL(paintRequested(QPrinter*)),
//                    this, SLOT(doPrint(QPrinter*)) );
//  printPreview->exec();
//  if(printDialog->exec()==QDialog::Accepted){
//    doPrint(printer);

    ui->pBar_Scan->setVisible( true );
    MFCDocumentPages *pages = m_Document->pages();
    int count = pages->count();
    ui->pBar_Scan->setMaximum(count);
    ui->pBar_Scan->setFormat(tr("Печать: %p%"));

    QPainter painter;
    painter.begin(&printer);

    for(int pIdx=0;pIdx<count;pIdx++){
      QPixmap pixmap = QPixmap();
      MFCDocumentPage *page = pages->getPage(pIdx);
      pixmap.loadFromData(page->getBody());
      painter.drawPixmap(
            0, 0, pixmap.scaled(printer.pageRect().size(),
                                Qt::KeepAspectRatio, Qt::SmoothTransformation));
      if(pIdx<count-1) printer.newPage();
      ui->pBar_Scan->setValue(pIdx*100/count);
    }

    painter.end();
    ui->pBar_Scan->setVisible( false );
    //  }

//  ui->pBar_Scan->setVisible(false);
//  delete printer;
//  delete printPreview;
    //    delete printDialog;

  MFCCore::execFile(pdf);
  QFile::remove(pdf);
}

void ElectroDoc_v2::doPrint(QPrinter *printer){
  viewer->print(printer);
}

void ElectroDoc_v2::save(){
  if(!ui->tBt_SaveDocument->isEnabled()) return;

  QString needFillText;
  if(ui->cBox_DocType->currentIndex()==-1) needFillText=ui->label_1->text()+"\n";
  if(ui->dEdit_DocDate->date()==ui->dEdit_DocDate->minimumDate())
    needFillText+=ui->label_3->text()+"\n";
  if(m_Document==NULL ||
     (!m_Document->havePages() && !m_Document->haveAttachments()))
    needFillText+=tr("Сканированная копия либо вложение");
  if(needFillText.length()>0){
    QMessageBox::warning(this,tr("Внимание!"),
                         tr("Для сохранения документа необходимо заполнить\n\n")+
                         needFillText);
    return;
  }

  needFillText.clear();
  if(ui->cBox_DocType->findText(ui->cBox_DocType->currentText())<0)
    needFillText=ui->label_1->text()+"\n";
  if( ui->cBox_DocAgency->currentIndex() > -1 &&
      ui->cBox_DocAgency->findText( ui->cBox_DocAgency->currentText() ) < 0 )
    needFillText+=ui->label_5->text()+"\n";
  if(!needFillText.isEmpty()){
    QMessageBox::warning(this,tr("Внимание!"),
                         tr("Ошибка в полях:\n")+needFillText+
                         tr("Необходимо выбрать значение из соответствующего списка"));
    return;
  }

  if(ui->lEdit_DocNum->text().isEmpty()){
    QString what=QString();
    if(ui->lEdit_DocNum->text().isEmpty()) what=tr("№");
    //    if(ui->lEdit_DocSer->text().isEmpty()) what+=(what.isEmpty()?"":tr(" и "))+tr("серия");
    if(!what.isEmpty()) what+=tr(" документа");
    QMessageBox *msg = new QMessageBox( QMessageBox::Question,tr("Внимание"),
                                        tr("%1 пуст. Уверены что хотите сохранить документ?").arg(what),
                                        QMessageBox::NoButton, this );
    msg->addButton(tr("Да"),QMessageBox::AcceptRole);
    msg->addButton(tr("Нет"),QMessageBox::RejectRole);
    msg->setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint );
    msg->setAttribute( Qt::WA_DeleteOnClose );
    msg->setStyleSheet( tr( "QLabel{border: none; background: none;color: black;}"
                            "QAbstractButton{min-width: 50px}" ) );
    if(msg->exec()==1){
      msg = NULL;
      return;
    }
    msg = NULL;
  }
  ui->tBt_SaveDocument->setEnabled(false);
  qApp->processEvents();
  m_Document->setType( ui->cBox_DocType->currentText() );
  m_Document->setName( ui->lEdit_DocName->text().simplified() );
  m_Document->setSeries( ui->lEdit_DocSer->text().simplified() );
  m_Document->setNumber( ui->lEdit_DocNum->text().simplified() );
  m_Document->setDate( ( ui->dEdit_DocDate->date() > QDate( 1800, 1, 1 ) ?
                           ui->dEdit_DocDate->date() : QDate() ) );
  m_Document->setExpiresDate( ( ui->dEdit_DocExpires->date() > QDate( 1800, 1, 1 ) ?
                                  ui->dEdit_DocExpires->date() : QDate() ) );
//  LogDebug()<<"m__State ="<<m__State;
  if(m__State==Copy || m__State==CopyCreate)
    m_Document->setCreateDate(QDateTime::currentDateTime());
  if(ui->cBox_DocAgency->currentIndex()>-1){
    m_Document->setAgency( ui->cBox_DocAgency->currentText() );
  }

//  hide();
  if(originalDocument==NULL){
    originalDocument=MFCDocument::instance();
//    originalDocument->setProperty("created_in",tr("%1 (%2)").arg(__FILE__).arg(__LINE__));
  }
  if(isModified() && !isReadOnly) originalDocument->copyFrom(m_Document);
//  LogDebug()<<"CreateDate ="<<m_Document->createDate().toString("dd.MM.yyyy hh:mm:ss.zzz")<<originalDocument->createDate().toString("dd.MM.yyyy hh:mm:ss.zzz");
  setModified(false);
  canJustClose=true;
  saved=true;
  loopResult = 0;
  close();
}

void ElectroDoc_v2::confirm(){
  if(originalDocument==NULL){
    originalDocument=MFCDocument::instance();
//    originalDocument->setProperty("created_in",tr("%1 (%2)").arg(__FILE__).arg(__LINE__));
    if(!m_Document){
      setError(tr("Ссылка на документ пуста"));
      MFCDocument::remove(originalDocument);
      originalDocument=NULL;
    }else
      originalDocument->copyFrom(m_Document);
  }
  canJustClose=true;
  saved=true;
  loopResult = 0;
//  hide();
  close();
}

void ElectroDoc_v2::reject(){
  canJustClose=true;
  if ( m__State == Read ) loopResult = 0;
  emit rejected();
//  hide();
  close();
}

void ElectroDoc_v2::fileChanged(QString fName){
  if(isReadOnly || !m_Document) return;
  QFileInfo fi(fName);
  if(fi.exists()){
    m_Document->attachments()->removeAttachment(fi.fileName());
    loadExtFile(fName);
    setModified(true);
  }
}

void ElectroDoc_v2::openAttachment(){
  if(!m_Document) return;
  if(ui->lstWgt_Attachments->selectedItems().count()==0) return;
  if(tmpFileName.length()>0) QFile::remove(tmpFileName);
  foreach(QListWidgetItem* item,ui->lstWgt_Attachments->selectedItems()){
    QString fileName=item->text();
    tmpFileName=QDir::tempPath()+"/"+fileName;
    QFile f(tmpFileName);
    f.open(QFile::WriteOnly);
    f.write(m_Document->attachments()->getAttachment(fileName).data());
    f.close();
    fWatcher->addPath(tmpFileName);
    QProcess p(this);
    QString exeName;
    QString opSys;
#ifdef Q_OS_WIN
    exeName="cmd /C \""+tmpFileName+"\"";
    opSys="WIN32";
#endif
#ifdef Q_OS_LINUX
    exeName="xdg-open \""+tmpFileName+"\"";
    opSys="LINUX";
#endif
    p.setProcessChannelMode(QProcess::MergedChannels);
    if(p.execute(exeName)!=0){
      QByteArray out=p.readAll();
#ifdef Q_OS_WIN
      out+="\ncmd /C "+QDir::toNativeSeparators(tmpFileName).toLocal8Bit();
      QTextDecoder decoder(QTextCodec::codecForName("Windows-1251"));
#endif
      QMessageBox::warning(
            this,tr("Ошибка запуска внешней программы"),
            tr("Возможно у Вас в системе отсутствует обработчик данного типа файлов. "
               "Сообщите данные, указанные ниже, ИТ специалисту.\n\nFileName: ")+
            fileName+"\nMIMEType: "+
            m_Document->attachments()->getAttachment(fileName).mimeType()+
            "\nOperating system: "+opSys+
            "\nCommand: "+exeName+"\nCommand output:\n"+
#ifdef Q_OS_WIN
            decoder.toUnicode(out).toLocal8Bit());
#else
      out);
#endif
    }
    fWatcher->removePath(tmpFileName);
#ifdef Q_OS_WIN
    f.remove(exeName);
#endif
  }
}

void ElectroDoc_v2::setError(const QString &str){
  errStr="ElectroDoc_v2: "+str;
  qCritical()<<errStr;
  emit error(errStr);
}

void ElectroDoc_v2::on_tBt_DateTo_clicked()
{
  ui->dEdit_DocDate->setDate( QDate::currentDate() );
}

void ElectroDoc_v2::on_tBt_ExpiresTo_clicked()
{
  ui->dEdit_DocExpires->setDate( QDate::currentDate() );
}

void ElectroDoc_v2::on_tBt_ExpiresClear_clicked()
{
  ui->dEdit_DocExpires->setDate( ui->dEdit_DocExpires->minimumDate() );
}
