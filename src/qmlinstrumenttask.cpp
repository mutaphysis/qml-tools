#include "qmlinstrumenttask.h"

#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>

#include <private/qqmlscript_p.h>

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

bool QmlInstrumentTask::instrumentFolder(const QString &in, const QString &out)
{
    QDir inDir(in);
    QDir outDir(out); QFileInfo outDirInfo(out);

    if (!outDir.exists() || !outDirInfo.isWritable()) {
        qCritical() << "Cannot write to folder" << out;
        return false;
    }

    QList<QFileInfo> entries;
    QDirIterator iterator(inDir, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        QFileInfo fileinfo = iterator.fileInfo();
        if (!fileinfo.isDir()) {
            if (fileinfo.suffix() == "qml" /*|| fileinfo.suffix() == "js"*/) {
                entries << fileinfo;
            }
        }
    }

    foreach (QFileInfo fileinfo, entries) {
        bool okay = outDir.mkpath(fileinfo.path());

        if (!okay) {
            qCritical() << "Could not create folder" << fileinfo.absolutePath();
            return false;
        }

        okay = instrumentFile(fileinfo.filePath(), outDir.filePath(fileinfo.filePath()));

        if (!okay) {
            return false;
        }
    }

    return true;
}

bool QmlInstrumentTask::instrumentFile(const QString &in, const QString &out)
{
    bool okay;
    QString data = instrumentFile(in, okay);

    if (!okay) {
        return false;
    }

    QFile outFile(out);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "Cannot write" << out << ":" << outFile.errorString();
        return false;
    }

    QTextStream stream( &outFile );
    stream << data;
    outFile.close();

    qDebug() << "Wrote" << out << "successfully";

    return true;
}


QString QmlInstrumentTask::instrumentFile(const QString &filename, bool &okay)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << filename << ":" << file.errorString();
        okay = false;
        return QString();
    }

    QString code = QTextStream(&file).readAll();
    file.close();

    if (filename.endsWith(".qml")) {
        return instrumentQml(code, filename, okay);
    } else if (filename.endsWith(".js")) {
        return instrumentJs(code, filename, okay);
    }

    qCritical() << "Cannot handle" << filename;
    okay = false;
    return QString();
}

QString QmlInstrumentTask::instrumentQml(const QString &code, const QString &filename, bool &okay)
{
    ScriptCollector collector;
    bool parseOkay = collector.parse(code, QUrl::fromLocalFile(filename), filename);

    if (!parseOkay) {
        qCritical() << "Failed parsing" << filename << collector.errors();
        okay = false;
        return QString();
    }

    QList<ScriptCollector::Script> scripts = collector.scripts();

    QList<Replacement> replacements;
    QStringList properties; properties << "";
    foreach (ScriptCollector::Script script, scripts) {

        QString jsCode = script.code;

        JsInstrument::Instrumented instrumented = m_instrumenter->instrument(
                    jsCode,
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

    okay = true;
    return results;
}

QString QmlInstrumentTask::instrumentJs(const QString &code, const QString &filename, bool &okay)
{
    QString jsCode = code;

    // remove pragma and imports
    QQmlError *error = 0;
    QQmlScript::Parser::JavaScriptMetaData metadata = QQmlScript::Parser::extractMetaData(jsCode, error);

    if (error) {
        qCritical() << "Failed parsing" << filename << error->toString();
        okay = false;
        return QString();
    }

    // no js allowed before & between pragmas and imports
    quint32 writeableLocation = 0;
    foreach (QQmlScript::Import import, metadata.imports) {
        writeableLocation = qMax(import.location.range.offset + import.location.range.length, writeableLocation);
    }

    // no access to the actual parsed locations of pragmas
    QRegularExpression pragmaFinder(".pragma.+library");
    QRegularExpressionMatch match = pragmaFinder.match(code, writeableLocation);
    while (match.hasMatch()) {
        writeableLocation = match.capturedEnd();
        match = pragmaFinder.match(code, writeableLocation);
    }

    jsCode = code.mid(writeableLocation);
    quint16 line, column;
    ScriptCollector::mapOffsetToLineAndColumn(code, writeableLocation, line, column);
    JsInstrument::Instrumented instrumented = m_instrumenter->instrument(
                jsCode,
                filename,
                line,
                column);

    if (instrumented.code.isEmpty() || instrumented.preamble.isEmpty()) {
        qCritical() << "Error instrumenting" << filename << jsCode;
        okay = false;
        return QString();
    }

    QStringList results;
    results << code.mid(0, writeableLocation);
    results << ".import QtCov 1.0 as QtCov"; // no semicolon for imports in js
    results << instrumented.preamble;
    results << instrumented.code;

    QString result = results.join("\n\n");
    okay = true;
    return result;
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

