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

    static QJSValue loadCoverageData(QJSEngine *scriptEngine);
    static bool saveCoverageData(QJSEngine *scriptEngine);

private slots:
    void save();

private:
    static QString coverageFilePath();
    static bool jsStringify(QJSEngine *engine, const QJSValue &value, QString &content);
    static bool jsParse(QJSEngine *engine, const QString &content, QJSValue &value);
};

#endif // QML_COV_PLUGIN_H
