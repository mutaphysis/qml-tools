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
    quint32 offset;
    quint32 length;
    QString replacement;
};

bool replacementComparator(const Replacement &left, const Replacement &right) {
  return left.offset < right.offset;
}

QmlInstrumentTask::QmlInstrumentTask(QObject *parent) :
    QObject(parent),
    m_instrumenter(new JsInstrument(this))
{
}

bool QmlInstrumentTask::instrument(const QString &in, const QString &out)
{
    QFileInfo inDirInfo(in);

    if (!inDirInfo.exists() || !inDirInfo.isReadable()) {
        qCritical() << "Cannot read from" << in;
        return false;
    }

    // no out path specified, use in path
    QString outPath = out;
    if (out.isEmpty()) {
        outPath = in;
    }

    if (inDirInfo.isDir()) {
        return instrumentFolder(in, outPath);
    } else {
        return instrumentFile(in, outPath);
    }
}

bool QmlInstrumentTask::instrumentFolder(const QString &in, const QString &out)
{
    QDir inDir(in);
    QDir outDir(out);

    QList<QFileInfo> entries;
    QDirIterator iterator(inDir, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        QFileInfo fileinfo = iterator.fileInfo();
        if (!fileinfo.isDir()) {
            if (fileinfo.suffix() == "qml" || fileinfo.suffix() == "js") {
                entries << fileinfo;
            }
        }
    }

    foreach (QFileInfo fileinfo, entries) {

        QFileInfo relativePath(fileinfo.canonicalFilePath().mid(inDir.canonicalPath().length() + 1));

        bool okay = outDir.mkpath(relativePath.path());
        if (!okay) {
            qCritical() << "Could not create folder" << relativePath.dir();
            return false;
        }

        okay = instrumentFile(fileinfo.filePath(), outDir.filePath(relativePath.filePath()));

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

    QFileInfo fileInfo(filename);

    if (filename.endsWith(".qml")) {
        return instrumentQml(code, fileInfo.absoluteFilePath(), okay);
    } else if (filename.endsWith(".js")) {
        return instrumentJs(code, fileInfo.absoluteFilePath(), okay);
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

            Replacement scriptReplacement = {
                script.location.range.offset,
                script.location.range.length,
                instrumented.code
            };

            Replacement propertyReplacement = {
                script.owningObjectStartOffset,
                0,
                "\n" + instrumented.property + "\n"
            };

            replacements << propertyReplacement << scriptReplacement;
        } else {
            qCritical() << "Error instrumenting" << filename << script << script.code;
        }

    }

    QString results = rewrite(code, replacements);
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

QString QmlInstrumentTask::initialCoverageData()
{
    return m_instrumenter->initialCoverageData();
}

bool QmlInstrumentTask::saveInitialCoverageData(const QString &out)
{
    QFile outFile(out);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "Cannot write" << out << ":" << outFile.errorString();
        return false;
    }

    QTextStream stream( &outFile );
    stream << initialCoverageData();
    outFile.close();

    qDebug() << "Wrote" << out << "successfully";
    return true;
}


QString QmlInstrumentTask::rewrite(
        const QString &originalContents,
        const QList<Replacement> &creplacements)
{
    int offset = 0;

    QString replacedContents = originalContents;

    QList<Replacement> replacements = creplacements;
    qSort(replacements.begin(), replacements.end(), replacementComparator);


    foreach (Replacement replacement, replacements) {
        // qDebug() << "rewriting" << replacement.offset;
        // qDebug() << replacedContents.mid(replacement.offset + offset, replacement.length);
        // qDebug() << "with\n" << replacement.replacement;

        replacedContents.replace(
                    replacement.offset + offset,
                    replacement.length,
                    replacement.replacement);


        offset += replacement.replacement.length() - replacement.length;
    }

    return replacedContents;
}

