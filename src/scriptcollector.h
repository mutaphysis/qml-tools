#ifndef SCRIPTCOLLECTOR_H
#define SCRIPTCOLLECTOR_H

#include <QByteArray>
#include <QQmlError>
#include <QList>
#include <QDebug>

namespace QQmlScript {
 class Object;
}

/**
 * The ScriptCollector parses a qml file (just parsing no compiling) and collects
 * all occurrences of javascript in this file.
 *
 * These are
 *  - property declarations
 *  - property definitions
 *  - signal handlers
 *  - functions
 *  - all of these in objects in properties and in the default property
 *
 * There is no detection for properties that will be QQmlV4Bindings or QQmlV8Bindings,
 * they will all be treated as javascript.
 *
 */
class ScriptCollector
{
public:
    enum Type {
        Property,
        Function,
        AnonymousFunctionProperty
    };

    struct LocationRange
    {
        quint32 offset;
        quint32 length;
    };

    struct Location
    {
        quint16 line;
        quint16 column;

        inline bool operator<(const Location &other)
        {
            return line < other.line ||
                   (line == other.line && column < other.column);
        }
    };

    struct LocationSpan
    {
        Location start;
        Location end;
        LocationRange range;

        inline bool operator<(const LocationSpan &o) const
        {
            return (start.line < o.start.line) ||
                   (start.line == o.start.line && start.column < o.start.column);
        }
    };

    struct Script {
        QString name;
        QString code;
        Type type;
        LocationSpan location;

        bool operator<(const Script &o) const
        {
            return (location < o.location);
        }
    };

    ScriptCollector();
    virtual ~ScriptCollector();

    bool parse(const QString &data, const QUrl &url, const QString &urlString);
    void clear();

    QList<ScriptCollector::Script> scripts() const;
    QList<QQmlError> errors() const;
    quint32 objectStartOffset() const;

    static void mapOffsetToLineAndColumn(const QString &data, const quint32 &offset, quint16 &line, quint16 &column);
private:
    void determineObjectStartOffset(const QString &data, QQmlScript::Object *node);
    void collectJS(QQmlScript::Object *node, const QString &data);

    QList<QQmlError> m_errors;
    QList<ScriptCollector::Script> m_scripts;
    quint32 m_objectStartOffset;
};

QDebug operator<<(QDebug dbg, const ScriptCollector::Script &script);

#endif // SCRIPTCOLLECTOR_H
