#include "jslint.h"

#include <QFile>
#include <QDebug>

JsLint::JsLint(QObject *parent)
    : QObject(parent)
{
}

JsLint::~JsLint()
{
}

bool JsLint::setup(const QString &lintPath)
{
    QFile lintFile(lintPath);
    if (!lintFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << lintPath << ":" << lintFile.errorString();
        return false;
    }

    QString jslintCode = QTextStream(&lintFile).readAll();
    QJSValue lintLoader = m_engine.evaluate(jslintCode, lintPath);

    if (lintLoader.isError()) {
        qCritical() << "Uncaught exception:" << lintLoader.toString();
        return false;
    }

    m_lintFunction = m_engine.evaluate("JSLINT");

    if (m_lintFunction.isError()) {
        qCritical() << "Uncaught exception:" << m_lintFunction.toString();
        m_lintFunction = QJSValue();
        return false;
    }

    return true;
}

QJsonDocument JsLint::lint(const QString &code)
{
    QJSValueList args;
    args << code;

    QJSValue lintResult = m_lintFunction.call(args);

    qDebug() << "JsLint okay:" << lintResult.toBool();

    QJSValue jsLintErrors = m_engine.evaluate("JSON.stringify(JSLINT.errors)");
    if (jsLintErrors.isError()) {
        qCritical() << "Uncaught exception:" << jsLintErrors.toString();
        return QJsonDocument();
    }

    QJsonParseError error;
    QJsonDocument results = QJsonDocument::fromJson(jsLintErrors.toString().toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        qCritical() << "Could not parse results:" << error.errorString();
        return QJsonDocument();
    }

    return results;
}
