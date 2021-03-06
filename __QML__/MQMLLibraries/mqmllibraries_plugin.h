#ifndef MQMLLIBRARIES_PLUGIN_H
#define MQMLLIBRARIES_PLUGIN_H

#include <QQmlExtensionPlugin>
#include <QtQml>

class MQMLLibrariesPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void initializeEngine( QQmlEngine *engine, const char *uri );
    void registerTypes(const char *uri);
};

#endif // MQMLLIBRARIES_PLUGIN_H

