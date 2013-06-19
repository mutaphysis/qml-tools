#ifndef QML_COV_PLUGIN_H
#define QML_COV_PLUGIN_H

#include <QQmlExtensionPlugin>
#include <QQmlEngine>

class QmlCovPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "QtCov")
    
public:
    void registerTypes(const char *uri);
    void initializeEngine(QQmlEngine *engine, const char *uri);

private slots:
    void aboutToQuit();
};

#endif // QML_COV_PLUGIN_H
