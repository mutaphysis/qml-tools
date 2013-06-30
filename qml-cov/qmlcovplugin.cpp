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
        g_coverageData = scriptEngine->newObject();
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

    qDebug() << coverageFilePath();

    connect(QCoreApplication::instance(),
            SIGNAL(aboutToQuit()),
            SLOT(aboutToQuit()),
            Qt::DirectConnection);
}

void QmlCovPlugin::aboutToQuit()
{
    QJSValue JSON = g_engine->evaluate("JSON");

    if (JSON.isError()) {
        qCritical() << "Uncaught exception:" << JSON.toString();
        return;
    }

    QJSValue stringify = JSON.property("stringify");

    if (stringify.isError()) {
        qCritical() << "Uncaught exception:" << stringify.toString();
        return;
    }

    QJSValue results = stringify.callWithInstance(JSON, QJSValueList() << g_coverageData);

    if (results.isError()) {
        qCritical() << "Uncaught exception:" << results.toString();
        return;
    }

    qDebug().nospace() << "\n" << qPrintable(results.toString());
}

QString QmlCovPlugin::coverageFilePath()
{
    QString envPath = qgetenv("QTCOV_COVERAGE_DATA_PATH");

    if (!envPath.isEmpty()) {
        QFileInfo envInfo(envPath);
        if (envInfo.exists() && envInfo.isReadable() && envInfo.isWritable()) {
            return envPath;
        }
        qCritical() << "Could not use configuration from" << envPath;
    }

    QString defaultName = "coverage_data.json";

    QFileInfo currentDirInfo(defaultName);
    if (currentDirInfo.exists() && currentDirInfo.isReadable() && currentDirInfo.isWritable()) {
        return QDir::current().absoluteFilePath(defaultName);
    } else {
        qCritical() << "Could not use configuration from" << QDir::current().absoluteFilePath(defaultName);
    }

    QFileInfo appDirInfo(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(defaultName));
    if (appDirInfo.exists() && appDirInfo.isReadable() && appDirInfo.isWritable()) {
        return appDirInfo.absoluteFilePath();
    } else {
        qCritical() << "Could not use configuration from" << appDirInfo.absoluteFilePath();
    }

    return defaultName;
}

