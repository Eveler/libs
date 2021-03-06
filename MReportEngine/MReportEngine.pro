QT       += core xml sql network

TARGET = MReportEngine
CONFIG(debug, debug|release): TARGET = $${TARGET}d
#CONFIG += create_prl

TEMPLATE = lib

DEFINES += \
    EXPORT_LIB_MREPORTENGINE


exists( ../install_path.pri ){
    include(../install_path.pri)
}

INCLUDEPATH += ./ \
    ./MReportDocument/ \
    ./MReportDocument/MReportSource/ \
    ./MReportDocument/MReportParameter/ \
    ./MReportDocument/MReportKey/ \
    ./MReportDocument/MReport/ \
    ../include/ \
    ../3dparty/quazip/quazip/

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE279790D
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = MReportEngine.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {

    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

win32 {
    target.path = $$INSTALL_WIN_LIB
    INSTALLS += target
}

#greaterThan(QT_MAJOR_VERSION, 4) {
#  DESTDIR = ../bin_qt5
#  DLLDESTDIR = ../bin_qt5

#  LIBS += -L../bin_qt5/ \
#      -lmuCalculator \
#      -lAMSLogger
#}

LIB_LIST = \
    -lMFCCore \
    -lquazip \
    -lAMSLogger \
    -lmuCalculator

greaterThan( QT_MAJOR_VERSION, 4 ) {
  DESTDIR = ../bin_qt5
  DLLDESTDIR = ../bin_qt5

  LIBS += -L../bin_qt5/ \
      $${LIB_LIST}
}
lessThan( QT_MAJOR_VERSION, 5 ) {
  DESTDIR = ../bin
  DLLDESTDIR = ../bin

  LIBS += -L../bin/ \
      $$LIB_LIST
}


CONFIG(release, debug|release){
  OBJECTS_DIR = ./temp/$$TARGET/release
  MOC_DIR = ./temp/$$TARGET/release
  RCC_DIR = ./temp/$$TARGET/release
  UI_DIR = ./temp/$$TARGET/release
}
CONFIG(debug, debug|release){
  OBJECTS_DIR = ./temp/$$TARGET/debug
  MOC_DIR = ./temp/$$TARGET/debug
  RCC_DIR = ./temp/$$TARGET/debug
  UI_DIR = ./temp/$$TARGET/debug
}

HEADERS += \
    ../include/export/mreport_engine_export.h \
    ../include/mreportdocument.h \
    MReportDocument/mreportdocument_p.h \
    ../include/mreportsource.h \
    MReportDocument/MReportSource/mreportsource_p.h \
    ../include/mreportparameter.h \
    MReportDocument/MReportParameter/mreportparameter_p.h \
    ../include/mreportkey.h \
    MReportDocument/MReportKey/mreportkey_p.h \
    MReportDocument/MReport/mreport.h

SOURCES += \
    MReportDocument/mreportdocument.cpp \
    MReportDocument/mreportdocument_p.cpp \
    MReportDocument/MReportSource/mreportsource.cpp \
    MReportDocument/MReportSource/mreportsource_p.cpp \
    MReportDocument/MReportParameter/mreportparameter.cpp \
    MReportDocument/MReportParameter/mreportparameter_p.cpp \
    MReportDocument/MReportKey/mreportkey.cpp \
    MReportDocument/MReportKey/mreportkey_p.cpp \
    MReportDocument/MReport/mreport.cpp

