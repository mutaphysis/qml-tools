#include "scriptcollector.h"

#include <QDebug>

#include <private/qqmldata_p.h>
#include <private/qqmltypeloader_p.h>
#include <private/qqmlscript_p.h>

ScriptCollector::ScriptCollector()
{
}

ScriptCollector::~ScriptCollector()
{
}

bool ScriptCollector::parse(const QString &data,
                            const QUrl &url = QUrl(),
                            const QString &urlString = QString())
{
    QQmlScript::Parser parser;
    parser.parse(data, QByteArray(), url, urlString);

    m_errors += parser.errors();

    if (parser.errors().isEmpty()) {
        qDebug() << "Parsed" << urlString << "succesfully";
        collectJS(parser.tree());
        qSort(m_scripts);
        return true;
    } else {
        qCritical() << parser.errors();
        return false;
    }

}

void ScriptCollector::clear()
{
    m_errors.clear();
    m_scripts.clear();
}

QList<ScriptCollector::Script> ScriptCollector::scripts() const
{
    return m_scripts;
}

inline QString toString(const QHashedStringRef& ref)
{
    return QString(ref.constData(), ref.length());
}

void ScriptCollector::collectJS(QQmlScript::Object *node)
{
    // main properties
    QQmlScript::Property *prop = node->properties.first();
    while (prop != 0) {

        QQmlScript::Value *value = prop->values.first();
        while (value != 0) {

            if (toString(prop->name()) != "id") {
                if (value->object) {
                    // properties with qml objects
                    collectJS(value->object); // recurse
                } else if (value->value.isScript()) {

                    // qDebug().nospace() << ">>>>> " << toString(prop->name()) << " ("
                    //                   << value->location.start.line << ":" << value->location.start.column << " to "
                    //                   << value->location.end.line << ":" << value->location.end.column << ")";

                    // Javascript ->
                    // qDebug() << value->value.asScript();

                    Script script = {toString(prop->name()),
                                     value->value.asScript(),
                                     ScriptCollector::Property,
                                     { { value->location.start.line, value->location.start.column },
                                       { value->location.end.line, value->location.end.column } } };
                    m_scripts.append(script);
                }
            }

            value = prop->values.next(value);
        }

        prop = node->properties.next(prop);
    }

    // child-objects
    if (node->defaultProperty) {

        QQmlScript::Value *value = node->defaultProperty->values.first();
        while (value != 0) {

            if (value->object) {
                collectJS(value->object); // recurse
            }
            value = node->defaultProperty->values.next(value);
        }
    }

    // all other properties
    QQmlScript::Object::DynamicProperty *dprop = node->dynamicProperties.first();
    while (dprop != 0) {

        bool isAlias = (dprop->type == QQmlScript::Object::DynamicProperty::Alias);

        QQmlScript::Property *prop = dprop->defaultValue;
        if (prop && !isAlias) {

            QQmlScript::Value *value = prop->values.first();
            while (value != 0) {

                if (value->value.isScript()) {
                    // qDebug().nospace() << ">>>>> " << toString(dprop->name) << " ("
                    //                   << value->location.start.line << ":" << value->location.start.column << " to "
                    //                   << value->location.end.line << ":" << value->location.end.column << ")";

                    // Javascript ->
                    // qDebug() << value->value.asScript();

                    Script script = {toString(dprop->name),
                                     value->value.asScript(),
                                     ScriptCollector::Property,
                                     { { value->location.start.line, value->location.start.column },
                                       { value->location.end.line, value->location.end.column } } };
                    m_scripts.append(script);
                }

                value = prop->values.next(value);
            }
        }

        dprop = node->dynamicProperties.next(dprop);
    }

    // functions
    QQmlScript::Object::DynamicSlot *dslot = node->dynamicSlots.first();
    while (dslot != 0) {
        // qDebug().nospace() << ">>>>> " << toString(dslot->name) << " ("
        //                   << dslot->location.start.line << ":" << dslot->location.start.column << " to "
        //                   << dslot->location.end.line << ":" << dslot->location.end.column << ")";

        // Javascript ->
        // qDebug() << dslot->body;

        Script script = {toString(dslot->name),
                         dslot->body,
                         ScriptCollector::Function,
                         { { dslot->location.start.line, dslot->location.start.column },
                           { dslot->location.end.line, dslot->location.end.column } } };
        m_scripts.append(script);

        dslot = node->dynamicSlots.next(dslot);
    }
}

QDebug operator<<(QDebug dbg, const ScriptCollector::Script &script)
{
    dbg.nospace() << script.name << " ("
                  << script.location.start.line << ":" << script.location.start.column << "-"
                  << script.location.end.line << ":" << script.location.end.column << ")";

    return dbg.space();
}
