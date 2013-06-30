#include "qmlcovplugin.h"

#include <QCoreApplication>
#include <QQmlEngine>
#include <QFileInfo>
#include <QJSValue>
#include <QDebug>
#include <QDir>

#include <qqml.h>

QJSValue g_coverageData;
QQmlEngine *g_engine;

static QJSValue example_qjsvalue_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)

    QJSValue example = scriptEngine->newObject();
    if (g_coverageData.isUndefined()) {
        g_engine = engine;
        g_coverageData = QmlCovPlugin::loadCoverageData(scriptEngine);
        example.setProperty("data", g_coverageData);
    }
    return example;
}

void QmlCovPlugin::registerTypes(const char *uri)
{    
    qmlRegisterSingletonType(uri, 1, 0, "coverage", example_qjsvalue_singletontype_provider);
}

void QmlCovPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(engine);
    Q_UNUSED(uri);

    connect(QCoreApplication::instance(),
            SIGNAL(aboutToQuit()),
            SLOT(aboutToQuit()),
            Qt::DirectConnection);
}

void QmlCovPlugin::aboutToQuit()
{    
    bool okay = QmlCovPlugin::saveCoverageData(g_engine);

    if (okay) {
        qDebug() << "Saved coverage data";
    } else {
        qCritical() << "Could not save coverage data";
    }
}


bool QmlCovPlugin::jsStringify(QJSEngine *engine, const QJSValue &value, QString &content)
{
    QJSValue JSON = engine->evaluate("JSON");
    if (JSON.isError()) {
        qCritical() << "Uncaught exception:" << JSON.toString();
        return false;
    }

    QJSValue stringify = JSON.property("stringify");
    if (stringify.isError()) {
        qCritical() << "Uncaught exception:" << stringify.toString();
        return false;
    }

    QJSValue results = stringify.callWithInstance(JSON, QJSValueList() << value);
    if (results.isError()) {
        qCritical() << "Uncaught exception:" << results.toString();
        return false;
    }

    content = results.toString();

    return true;
}

bool QmlCovPlugin::jsParse(QJSEngine *engine, const QString &content, QJSValue &value)
{
    QJSValue JSON = engine->evaluate("JSON");
    if (JSON.isError()) {
        qCritical() << "Uncaught exception:" << JSON.toString();
        return false;
    }

    QJSValue parse = JSON.property("parse");
    if (parse.isError()) {
        qCritical() << "Uncaught exception:" << parse.toString();
        return false;
    }

    QJSValue results = parse.callWithInstance(JSON, QJSValueList() << content);
    if (results.isError()) {
        qCritical() << "Uncaught exception:" << results.toString();
        return false;
    }

    value = results;

    return true;
}

QString QmlCovPlugin::coverageFilePath()
{
    QString envPath = qgetenv("QTCOV_COVERAGE_DATA_PATH");

    if (!envPath.isEmpty()) {
        QFileInfo envInfo(envPath);
        if (envInfo.exists() && envInfo.isReadable() && envInfo.isWritable()) {
            return envPath;
        }
        qCritical() << "Could not use coverage data from" << envPath;
    }

    QString defaultName = "coverage_data.json";

    QFileInfo currentDirInfo(defaultName);
    if (currentDirInfo.exists() && currentDirInfo.isReadable() && currentDirInfo.isWritable()) {
        return QDir::current().absoluteFilePath(defaultName);
    } else {
        qWarning() << "Could not use coverage data from" << QDir::current().absoluteFilePath(defaultName);
    }

    QFileInfo appDirInfo(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(defaultName));
    if (appDirInfo.exists() && appDirInfo.isReadable() && appDirInfo.isWritable()) {
        return appDirInfo.absoluteFilePath();
    } else {
        qWarning() << "Could not use coverage data from" << appDirInfo.absoluteFilePath();
    }

    return defaultName;
}


bool QmlCovPlugin::saveCoverageData(QJSEngine *scriptEngine)
{
    QString path = QmlCovPlugin::coverageFilePath();

    QString content;
    bool okay = jsStringify(scriptEngine, g_coverageData, content);

    if (!okay) {
        return false;
    }

    QFile outFile(path);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "Cannot write" << path << ":" << outFile.errorString();
        return false;
    }

    QTextStream stream( &outFile );
    stream << content;
    outFile.close();

    return true;
}


QJSValue QmlCovPlugin::loadCoverageData(QJSEngine *scriptEngine)
{
    QString path = QmlCovPlugin::coverageFilePath();

    QFile file(path);

    if (file.open(QIODevice::ReadOnly)) {
        QString code = QTextStream(&file).readAll();
        file.close();

        QJSValue result;
        bool okay = QmlCovPlugin::jsParse(scriptEngine, code, result);

        if (okay) {
            return result;
        }
    }

    qWarning() << "Could not load initial coverage data, uncovered files won't be listed in results.";
    return scriptEngine->newObject();
}
