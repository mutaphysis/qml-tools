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


    return true;
}

JsInstrument::Instrumented JsInstrument::instrument(
        const QString &code,
        const QString &fileName,
        const uint lineOffset,
        const uint columnOffset)
{
    QJSValue instance = m_engine.evaluate("myInstrumenter = new Instrumenter({ coverageVariable: 'QtCov.coverage.data'})");
    if (instance.isError()) {
        qCritical() << "Uncaught exception:" << instance.toString();
        return Instrumented();
    }

    QJSValue instrumentFunc = m_engine.evaluate("myInstrumenter.instrumentSync");
    if (instrumentFunc.isError()) {
        qCritical() << "Uncaught exception:" << instrumentFunc.toString();
        return Instrumented();
    }

    // make sure the instrumentation gets the same offsets as the original code
    QString indentedCode = QString(lineOffset, '\n') + QString(columnOffset, ' ') + code;
    QJSValueList args = QJSValueList() << indentedCode << fileName;
    QJSValue results = instrumentFunc.callWithInstance(instance, args);

    if (results.isError()) {
        qCritical() << "Uncaught exception:" << results.toString();
        return Instrumented();
    }


    Instrumented result = {results.property("code").toString(),
                           results.property("property").toString(),
                           results.property("preamble").toString()};
    return result;
}
