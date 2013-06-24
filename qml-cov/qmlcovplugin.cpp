#include "qmlcovplugin.h"

#include <QCoreApplication>
#include <QQmlEngine>
#include <QJSValue>
#include <QDebug>

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

