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
 * @brief The ScriptCollector class
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
        Function
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
protected:

private:
    void collectJS(QQmlScript::Object *node);

    QList<QQmlError> m_errors;
    QList<ScriptCollector::Script> m_scripts;
};


QDebug operator<<(QDebug dbg, const ScriptCollector::Script &script);

#endif // SCRIPTCOLLECTOR_H
