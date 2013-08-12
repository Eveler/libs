#ifndef MQMLLIBS_PLUGIN_H
#define MQMLLIBS_PLUGIN_H

#include <QQmlExtensionPlugin>


class MQMLLibsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri);
};

#endif // MQMLLIBS_PLUGIN_H

