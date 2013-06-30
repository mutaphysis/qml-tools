#ifndef JSINSTRUMENT_H
#define JSINSTRUMENT_H

#include <QObject>
#include <QString>
#include <QJsonDocument>
#include <QJSEngine>

class JsInstrument : public QObject
{
public:
    struct Instrumented {
        QString code;
        QString property;
        QString preamble;
    };

    JsInstrument(QObject *parent = 0);
    virtual ~JsInstrument();

    JsInstrument::Instrumented instrument(const QString &code,
                                          const QString &fileName,
                                          const uint lineOffset = 0,
                                          const uint columnOffset = 0);

    QString initialCoverageData();
private:
    bool setup();

    QJSEngine m_engine;    
    QJSValue m_coverageData;
    QJSValue m_instrumenterInstance;
};

#endif // JSINSTRUMENT_H
