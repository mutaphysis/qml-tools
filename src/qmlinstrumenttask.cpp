#include "qmlinstrumenttask.h"

#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QFile>

#include "jsinstrument.h"
#include "scriptcollector.h"

struct Replacement {
    ScriptCollector::Script original;
    QString replacement;
};

QmlInstrumentTask::QmlInstrumentTask(QObject *parent) :
    QObject(parent),
    m_instrumenter(new JsInstrument(this))
{
}

void QmlInstrumentTask::instrumentFile(const QString &filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << filename << ":" << file.errorString();
        return;
    }
    QString code = QTextStream(&file).readAll();

    ScriptCollector collector;
    bool parseOkay = collector.parse(code, QUrl::fromLocalFile(filename), filename);

    if (!parseOkay) {
        qCritical() << "Failed parsing" << filename << collector.errors();
        return;
    }

    QList<ScriptCollector::Script> scripts = collector.scripts();
    QList<Replacement> replacements;
    QStringList properties;
    properties << "";
    foreach (ScriptCollector::Script script, scripts) {

        QString code = script.code;

        JsInstrument::Instrumented instrumented = m_instrumenter->instrument(
                    code,
                    filename +
                            ":" + QString::number(script.location.start.line) +
                            ":" + QString::number(script.location.start.column),
                    script.location.start.line,
                    script.location.start.column);

        if (!instrumented.code.isEmpty() && !instrumented.property.isEmpty()) {
            // make sure code is always surrounded by {}
            if (instrumented.code.at(0) != QStringLiteral("{")) {
                instrumented.code = QStringLiteral("{%1}").arg(instrumented.code);
            }

            Replacement replacement = { script, instrumented.code };
            replacements << replacement;
            properties << instrumented.property;
        } else {
            qCritical() << "Error instrumenting" << filename << script << script.code;
        }
    }

    properties << "\n";

    QString results = rewrite(code, replacements);

    results.replace(collector.objectStartOffset(), 0, properties.join("\n    "));
    results.replace(0, 0, "import QtCov 1.0 as QtCov; ");

    qDebug() << results;
}

QString QmlInstrumentTask::rewrite(
        const QString &originalContents,
        const QList<Replacement> &replacements)
{
    int offset = 0;

    QString replacedContents = originalContents;

    foreach (Replacement replacement, replacements) {
//        qDebug() << "rewriting" << replacement.original.location.range.offset;
//        qDebug() << replacedContents.mid(replacement.original.location.range.offset + offset, replacement.original.location.range.length);
//        qDebug() << "with\n" << replacement.replacement;
        replacedContents.replace(
                    replacement.original.location.range.offset + offset,
                    replacement.original.location.range.length,
                    replacement.replacement);
        offset += replacement.replacement.length() - replacement.original.location.range.length;
    }

    return replacedContents;
}
