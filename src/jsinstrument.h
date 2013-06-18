#ifndef JSINSTRUMENT_H
#define JSINSTRUMENT_H

#include <QObject>
#include <QString>
#include <QJsonDocument>
#include <QJSEngine>

class JsInstrument : public QObject
{
public:
    JsInstrument(QObject *parent = 0);
    virtual ~JsInstrument();

    QString instrument(const QString &code, const QString &fileName);

private:
    bool setup();

    QJSEngine m_engine;
};

#endif // JSINSTRUMENT_H
