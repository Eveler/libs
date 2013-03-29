QT       += gui

TARGET = Post
CONFIG(debug, debug|release): TARGET = $${TARGET}d
CONFIG += create_prl

TEMPLATE = lib

DEFINES += EXPORT_LIB_POST

INCLUDEPATH += ./ \
    ../include/ \
    ./Post/ \
    ./PostsStorage/

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE279790D
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = Office.dll
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

LIB_LIST = \
    -lMFCCore \

greaterThan(QT_MAJOR_VERSION, 4) {
  INCLUDEPATH += \
      ../bin_qt5

  DESTDIR = ../bin_qt5
  DLLDESTDIR = ../bin_qt5

  LIBS += -L../bin_qt5/ \
      $$LIB_LIST
}

lessThan(QT_MAJOR_VERSION, 5) {
  INCLUDEPATH += \
      ../bin

  DESTDIR = ../bin
  DLLDESTDIR = ../bin

  LIBS += -L../bin/ \
      $$LIB_LIST
}

CONFIG(release, debug|release){
  OBJECTS_DIR = ../temp/$$TARGET/release
  MOC_DIR = ../temp/$$TARGET/release
  RCC_DIR = ../temp/$$TARGET/release
  UI_DIR = ../temp/$$TARGET/release
}
CONFIG(debug, debug|release){
  OBJECTS_DIR = ../temp/$$TARGET/debug
  MOC_DIR = ../temp/$$TARGET/debug
  RCC_DIR = ../temp/$$TARGET/debug
  UI_DIR = ../temp/$$TARGET/debug
}

HEADERS += \
    ../include/export/post_export_lib.h \
    ../include/post.h \
    Post/post_p.h \
    ../include/postsstorage.h

SOURCES += \
    Post/post.cpp \
    Post/post_p.cpp \
    PostsStorage/postsstorage.cpp










