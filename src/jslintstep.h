#ifndef JSLINTSTEP_H
#define JSLINTSTEP_H

#include <QObject>
#include <QString>
#include <QJsonDocument>
#include <QJSEngine>

class JsLintStep : public QObject
{
public:
    JsLintStep(QObject *parent = 0);
    virtual ~JsLintStep();

    QJsonDocument lint(const QString &code);
    bool setup(const QString &lintPath = "jslint/jslint.js");

private:
    QJSEngine m_engine;
    QJSValue m_lintFunction;
};

#endif // JSLINTSTEP_H
