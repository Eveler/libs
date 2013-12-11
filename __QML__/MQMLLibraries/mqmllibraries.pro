TEMPLATE = lib
TARGET = MQMLLibraries
QT += qml quick
CONFIG += qt plugin

TARGET = $$qtLibraryTarget($$TARGET)
uri = extensions.mqmllibraries

# Input
SOURCES += \
    mqmllibraries_plugin.cpp \
    logreader.cpp \
    Tree/treeitem.cpp \
    Tree/treemodel.cpp \
    mqml.cpp

HEADERS += \
    mqmllibraries_plugin.h \
    logreader.h \
    Tree/treeitem.h \
    Tree/treemodel.h \
    mqml.h

OTHER_FILES = qmldir \
    Tree/TreeView/TreeView.qml \
    Tree/TreeView/TreeItemView.qml \
    Tree/TreeView/TreeItemsList.qml \
    DateInput/DateInput.qml

QT_INSTALL_QML = C:/Qt/Qt5.1.1/5.1.1/mingw48_32/qml

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}

win32 {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
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

RESOURCES +=