#include "jsinstrument.h"

#include <QFile>
#include <QDebug>

JsInstrument::JsInstrument(QObject *parent)
    : QObject(parent)
{
    setup();
}

JsInstrument::~JsInstrument()
{
}

bool JsInstrument::setup()
{
    // register window to allow loading escodegen.browser
    m_engine.evaluate("window = this");

    QFile esprimaFile("://esprima/esprima.js");
    if (!esprimaFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << esprimaFile.fileName() << ":" << esprimaFile.errorString();
        return false;
    }

    QString esprimaCode = QTextStream(&esprimaFile).readAll();
    QJSValue esprimaLoader = m_engine.evaluate(esprimaCode, esprimaFile.fileName());

    if (esprimaLoader.isError()) {
        qCritical() << "Uncaught exception:" << esprimaLoader.toString();
        return false;
    }    

    // -- //

    QFile escodegenFile("://escodegen/escodegen.browser.js");
    if (!escodegenFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << escodegenFile.fileName() << ":" << escodegenFile.errorString();
        return false;
    }

    QString escodegenCode = QTextStream(&escodegenFile).readAll();
    QJSValue escodegenLoader = m_engine.evaluate(escodegenCode, escodegenFile.fileName());

    if (escodegenLoader.isError()) {
        qCritical() << "Uncaught exception:" << escodegenLoader.toString();
        return false;
    }

    // -- //

    QFile istanbulFile("://istanbul/instrumenter.js");
    if (!istanbulFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << istanbulFile.fileName() << ":" << istanbulFile.errorString();
        return false;
    }

    QString istanbulCode = QTextStream(&istanbulFile).readAll();
    QJSValue istanbulLoader = m_engine.evaluate(istanbulCode, istanbulFile.fileName());

    if (istanbulLoader.isError()) {
        qCritical() << "Uncaught exception:" << istanbulLoader.toString();
        return false;
    }

    // create instrumenter

    m_instrumenterInstance = m_engine.evaluate("myInstrumenter = new Instrumenter({ coverageVariable: 'QtCov.coverage.data'})");
    if (m_instrumenterInstance.isError()) {
        qCritical() << "Create Instrumenter - Uncaught exception:" << m_instrumenterInstance.toString();
        return false;
    }


    // register coverage collector

    QJSValue qtCov = m_engine.newObject();
    QJSValue coverage = m_engine.newObject();
    m_coverageData = m_engine.newObject();
    m_engine.globalObject().setProperty("QtCov", qtCov);
    qtCov.setProperty("coverage", coverage);
    coverage.setProperty("data", m_coverageData);

    return true;
}

JsInstrument::Instrumented JsInstrument::instrument(
        const QString &code,
        const QString &fileName,
        const uint lineOffset,
        const uint columnOffset)
{
    QJSValue instrumentFunc = m_engine.evaluate("myInstrumenter.instrumentSync");
    if (instrumentFunc.isError()) {
        qCritical() << "Run Instrumentation - Uncaught exception:" << instrumentFunc.toString();
        return Instrumented();
    }

    // make sure the instrumentation gets the same offsets as the original code
    QString indentedCode = QString(lineOffset, '\n') + QString(columnOffset, ' ') + code;
    QJSValueList args = QJSValueList() << indentedCode << fileName;
    QJSValue results = instrumentFunc.callWithInstance(m_instrumenterInstance, args);

    if (results.isError()) {
        qCritical() << "Read Instrumented Data - Uncaught exception:" << results.toString();
        return Instrumented();
    }


    Instrumented result = {results.property("code").toString(),
                           results.property("property").toString(),
                           results.property("preamble").toString()};

    QJSValue storePreamble = m_engine.evaluate(result.preamble);

    if (storePreamble.isError()) {
        qCritical() << "Saving Coverage - Uncaught exception:" << storePreamble.toString();
    }

    return result;
}

QString JsInstrument::initialCoverageData()
{
    QJSValue readCoverage = m_engine.evaluate("JSON.stringify(QtCov.coverage.data)");

    if (readCoverage.isError()) {
        qCritical() << "Reading Coverage - Uncaught exception:" << readCoverage.toString();
        return QString();
    }

    return readCoverage.toString();
}
