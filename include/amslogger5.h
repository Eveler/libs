#ifndef AMSLOGGER_H
#define AMSLOGGER_H

#include <QSettings>
#include <QObject>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include "smtp.h"

#ifdef AMSLOGGER_BUILD
#define AMSLOGGER_EXPORT Q_DECL_EXPORT
#else
#define AMSLOGGER_EXPORT Q_DECL_IMPORT
#endif

/** \brief Выводит сообщения, передаваемые при помощи qDebug(), qWarning() и т.д.
  либо прямых вызов методов данного класса на консоль и/или файл. При выводе в
файл может "вращать" журналы
\author Савенко М.Ю.*/
class AMSLOGGER_EXPORT AMSLogger/*:QObject*/ {
//  Q_OBJECT
public:
  /** Уровень журналирования. По умолчания LevelCritical - ошибки
\see setLogLevel() \see logLevel()*/
  enum LogLevel {LevelDebug=1,LevelWarn=2,LevelCritical=4,LevelFatal=8};
  Q_DECLARE_FLAGS(LogLevels,LogLevel)
  Q_FLAGS(LogLevels)
  AMSLogger():msgFile(),msgLine(0){
    initialyze();
  }
  AMSLogger(QtMsgType type,QString &file,int line){
    initialyze();
    msgType=type;
    oldMsgType=-1;
    msgFile=file;
    msgLine=line;
  }
  ~AMSLogger(){QMessageLogContext c;messageOutput(msgType,c,"\n");}
  /** Выводит сообщение \param msg и записывает его в файл журнала в зависимости от
 установленного уровня на основании типа сообщения \param type.
\see setLogLevel()*/
  static void messageOutput(QtMsgType type, const QMessageLogContext &c=QMessageLogContext(),
                            const QString &msg=QString());
  /** Устанавливает обработчик сообщений, уровень журналирования в LevelCritical
 и имя файла журнала в
\bold qApp->applicationDirPath()+"/"+QFileInfo(qApp->applicationFilePath()).completeBaseName()+".log"
и "вращает" его \see uninstall()*/
  static void install();
  /** Возвращает системный обработчик сообщений \see install()*/
  static void uninstall(){
    qInstallMessageHandler(oldMsgHandler);
  }
  /** Устанавливает уровень журналирования в LevelCritical и имя файла журнала в
\bold qApp->applicationDirPath()+"/"+QFileInfo(qApp->applicationFilePath()).completeBaseName()+".log"
и "вращает" его*/
  static void initialyze();
  /** Записывает сообщение msg в файл журнала*/
  static void writeToFile(const QByteArray &msg);
  /** Устанавливает уровень журналирования в файл в \param level \see logLevel()*/
  static void setLogLevel(LogLevels level){
    loglevel=level;
  }
  /** Устанавливает количество хранимых файлов журнала при "вращении"*/
  static void setRotateCount(const int count){
    rotateCount=count;
  }
  /** Возвращает уровень журналирования в файл*/
  static LogLevels logLevel(){
    return loglevel;
  }
  /** Возвращает установлен ли AMSLogger в качестве обработчика QDebug*/
  static bool isInstalled(){
    return installed;
  }
  /** Выводит сообщение \param msg и записывает его в файл журнала в зависимости от
  установленного уровня на основании типа сообщения \param type.
  \see setLogLevel()*/
  AMSLogger& operator <<(const QVariant& msg);
  AMSLogger& operator<<(const void * ptr);


private:
  static void rotate();
  static QString completeBaseName();
  static QString prefix();

  static bool initialized;
  static bool installed;
  static LogLevels loglevel;
  static QFile *outFile;
  static QtMessageHandler oldMsgHandler;
  static QTextStream stream;
  static int oldMsgType;

  Smtp *smtp;
  static int rotateCount;
  QtMsgType msgType;
  QString msgFile;
  int msgLine;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AMSLogger::LogLevels)

AMSLOGGER_EXPORT AMSLogger logDebug(QString file,int line);
AMSLOGGER_EXPORT AMSLogger logWarning(QString file,int line);
AMSLOGGER_EXPORT AMSLogger logCritical(QString file,int line);
AMSLOGGER_EXPORT AMSLogger logFatal(QString file,int line);
#define LogFatal() logFatal(__FILE__,__LINE__)
#define LogCritical() logCritical(__FILE__,__LINE__)
#define LogWarning() logWarning(__FILE__,__LINE__)
#define LogDebug() logDebug(__FILE__,__LINE__)

#endif // AMSLOGGER_H