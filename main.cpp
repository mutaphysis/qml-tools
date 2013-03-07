#include <QCoreApplication>

#include <qqmljsparser_p.h>
#include <qqmljslexer_p.h>
#include <qqmljsastvisitor_p.h>
#include <qqmljsast_p.h>

#include <QtDebug>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QDirIterator>
#include <QStringList>

#include <iostream>


struct ComponentInfo {
    QString name;
    QString file;
    QString type;
    int column;
    int line;
    QString objectName;

    ComponentInfo(QString pname, QString pfile, QString ptype, int pcolumn, int pline, QString pobjectName=QString()) :
        name(pname), file(pfile), type(ptype), column(pcolumn), line(pline), objectName(pobjectName) {}
    ComponentInfo() {}
};


QMap<QString, ComponentInfo> g_componentInfos;
QMap<QString, QStringList> g_componentDependencies;

void static addToCache(ComponentInfo c) {
    if (!g_componentDependencies.contains(c.type)) {
        g_componentDependencies.insert(c.type, QStringList());
    }

    g_componentInfos.insert(c.name, c);
    g_componentDependencies[c.type].append(c.name);
}




class PropertyVisitor : protected QQmlJS::AST::Visitor {
public:
    PropertyVisitor() : m_deeper(true) {}

    virtual ~PropertyVisitor() {}

    QString operator()(const QString &propertyName, QQmlJS::AST::Node *node)
    {
        m_propertyName = propertyName;
        accept(node);
        return m_propertyValue;
    }

protected:
    using QQmlJS::AST::Visitor::visit;
    using QQmlJS::AST::Visitor::endVisit;

    void accept(QQmlJS::AST::Node *node)
    {
        QQmlJS::AST::Node::acceptChild(node, this);
    }

    // prevent going through the whole hierarchy
    virtual bool visit(QQmlJS::AST::UiObjectDefinition *) { if (m_deeper) { m_deeper = false; return true; } return m_deeper; }
    virtual bool visit(QQmlJS::AST::UiProgram *) { return false; }
    virtual bool visit(QQmlJS::AST::UiImportList *) { return false; }
    virtual bool visit(QQmlJS::AST::UiImport *) { return false; }
    virtual bool visit(QQmlJS::AST::UiPublicMember *) { return false; }
    virtual bool visit(QQmlJS::AST::UiSourceElement *) { return false; }
    virtual bool visit(QQmlJS::AST::UiObjectBinding *) { return false; }
    virtual bool visit(QQmlJS::AST::UiArrayBinding *) { return false; }
    virtual bool visit(QQmlJS::AST::UiArrayMemberList *) { return false; }
    virtual bool visit(QQmlJS::AST::UiQualifiedId *) { return false; }

    // find property values
    void endVisit(QQmlJS::AST::UiScriptBinding *prop)
    {
        const QString propertyName = asString(prop->qualifiedId);

        if (propertyName == m_propertyName) {
            if (QQmlJS::AST::ExpressionStatement* statement = QQmlJS::AST::cast<QQmlJS::AST::ExpressionStatement*>(prop->statement)) {
                if (QQmlJS::AST::StringLiteral* expression = QQmlJS::AST::cast<QQmlJS::AST::StringLiteral*>(statement->expression)) {
                    //qDebug() << "property" << m_propertyName << "defined with" << expression->value;
                    m_propertyValue = expression->value.toString();
                }
            }
        }
    }

    QString asString(QQmlJS::AST::UiQualifiedId *node) const
    {
        QString s;

        for (QQmlJS::AST::UiQualifiedId *it = node; it; it = it->next) {
            s.append(it->name.toString());

            if (it->next) {
                s.append(QLatin1Char('.'));
            }
        }

        return s;
    }

    QString m_propertyValue;
    QString m_propertyName;
    bool m_deeper;
};

class FindComponents : protected QQmlJS::AST::Visitor
{
public:
    FindComponents() {}

    virtual ~FindComponents() {}

    void operator()(const QString &fileName, QQmlJS::AST::Node *node)
    {
        m_fileName = fileName;
        m_component = QFileInfo(fileName).baseName();
        accept(node);
    }

protected:
    using QQmlJS::AST::Visitor::visit;
    using QQmlJS::AST::Visitor::endVisit;

    void accept(QQmlJS::AST::Node *node)
    {
        QQmlJS::AST::Node::acceptChild(node, this);
    }

    // find elements in properties
    void endVisit(QQmlJS::AST::UiObjectBinding *object) {
        const QString objectType = asString(object->qualifiedTypeNameId);
        const QQmlJS::AST::SourceLocation typeLocation = object->qualifiedTypeNameId->identifierToken;

        PropertyVisitor prop;
        QString objectName = prop("objectName", object);

        //qDebug() << "Inline object defined" << objectType << typeLocation.startLine;
        QString baseName = m_component + "::" + objectType + "_";
        m_components.append(ComponentInfo(baseName, m_fileName, objectType,
                                          typeLocation.startColumn, typeLocation.startLine, objectName));

    }

    // find child elements
    void endVisit(QQmlJS::AST::UiObjectDefinition *object)
    {
        const QString objectType = asString(object->qualifiedTypeNameId);
        const QQmlJS::AST::SourceLocation typeLocation = object->qualifiedTypeNameId->identifierToken;

        // Do not parse grouping properties (lowercase)
        if (objectType[0] != objectType[0].toUpper()) {
            return;
        }

        PropertyVisitor prop;
        QString objectName = prop("objectName", object);

        QString baseName = m_component + "::" + objectType + "_";
        m_components.append(ComponentInfo(baseName, m_fileName, objectType,
                                          typeLocation.startColumn, typeLocation.startLine, objectName));
        // qDebug() << "Object defined" << objectType << typeLocation.startLine;
    }

    void endVisit(QQmlJS::AST::UiProgram *) {
        ComponentInfo c = m_components.takeLast();
        // TODO ensure component name is unique (might break with same named files in namespaces/subfolders)
        c.name = m_component;
        addToCache(c);

        foreach (ComponentInfo c, m_components) {
            int uniqueNumber = 0;
            QString baseName = c.name;
            QString uniqueName;
            do {
                uniqueName = baseName + QString::number(uniqueNumber);
                uniqueNumber++;
            } while (g_componentInfos.contains(uniqueName));
            c.name = uniqueName;

            addToCache(c);
        }
    }

    QString asString(QQmlJS::AST::UiQualifiedId *node) const
    {
        QString s;

        for (QQmlJS::AST::UiQualifiedId *it = node; it; it = it->next) {
            s.append(it->name.toString());

            if (it->next) {
                s.append(QLatin1Char('.'));
            }
        }

        return s;
    }

private:
    QString m_fileName;
    QString m_component;
    QList<ComponentInfo> m_components;
};

QString createErrorString(const QString &filename, const QString &code, QQmlJS::Parser &parser)
{
    QStringList lines = code.split(QLatin1Char('\n'));
    lines.append(QLatin1String("\n"));
    QString errorString;

    foreach (const QQmlJS::DiagnosticMessage &m, parser.diagnosticMessages()) {
        if (m.isWarning()) {
            continue;
        }

        QString error = filename + QLatin1Char(':') + QString::number(m.loc.startLine)
                        + QLatin1Char(':') + QString::number(m.loc.startColumn) + QLatin1String(": error: ")
                        + m.message + QLatin1Char('\n');

        int line = 0;
        if (m.loc.startLine > 0) {
            line = m.loc.startLine - 1;
        }

        const QString textLine = lines.at(line);

        error += textLine + QLatin1Char('\n');

        int column = m.loc.startColumn - 1;
        if (column < 0)
            column = 0;

        column = qMin(column, textLine.length());

        for (int i = 0; i < column; ++i) {
            const QChar ch = textLine.at(i);
            if (ch.isSpace()) {
                error += ch.unicode();
            } else {
                error += QLatin1Char(' ');
            }
        }
        error += QLatin1String("^\n");
        errorString += error;
    }
    return errorString;
}

static bool loadQmlFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << filename << ":" << file.errorString();
        return false;
    }

    std::cout << "\rParsing " << filename.toStdString() << std::flush;

    QString code = QTextStream(&file).readAll();

    QQmlJS::Engine driver;
    QQmlJS::Parser parser(&driver);

    QQmlJS::Lexer lexer(&driver);
    lexer.setCode(code, /*line = */ 1, /*qmlMode*/ true);
    driver.setLexer(&lexer);

    if (parser.parse()) {
        //qDebug() << "Parsed" << filename << "succesfully";

        FindComponents finder;
        finder(filename, parser.rootNode());
    } else {
        QString error = createErrorString(filename, code, parser);
        qCritical() << error;
        return false;
    }
    std::cout << "\r" << std::flush;
    return true;
}

void printUsageGraph(QString componentName, int indent = 0) {
    if (componentName.contains(componentName)) {
        QStringList children = g_componentDependencies.value(componentName);
        QString indentString = QString(" ").repeated(indent);

        if (g_componentInfos.contains(componentName)) {
            ComponentInfo ci = g_componentInfos.value(componentName);

            QString name = indentString + ci.name;
            QString ciInfo = QString("file: %1:%2:%3").arg(ci.file).arg(ci.line).arg(ci.column);
            if (!ci.objectName.isEmpty()) {
                ciInfo += ", named: " + ci.objectName;
            }
            qDebug("%s (%s)", name.toStdString().c_str(), ciInfo.toStdString().c_str());
        } else {
            QString name = indentString + componentName;
            qDebug("%s (Unknown)", name.toStdString().c_str());
        }

        foreach (QString childName, children) {
            printUsageGraph(childName, indent + 4);
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString initialPath = ".";
    QStringList arguments = app.arguments();
    for (int i = 0; i < arguments.length(); i++) {
        QString s = arguments.at(i);

        if (s == "--help" || s == "-?" || s== "-h") {
            qDebug() << "Parsley : A qml inheritance parser\n";
            qDebug() << "Searches for all qml files in a directory and in all subdirectories.";
            qDebug() << "Prints the hierarchy of all objects or of a given subtree.";
            qDebug() << "\nArguments:";
            qDebug() << "   --help This text (-h, -?)";
            qDebug() << "   --path <path> The path to search recursively for qml files (-p)";
            exit(0);
        } else if (s == "--path" || s == "-p") {
            if (++i < arguments.length()) {
                initialPath = arguments.at(i);
            } else {
                qCritical("Not enough arguments for --path or -p");
                exit(-1);
            }
        }
    }

    QDirIterator walker(initialPath, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while(walker.hasNext()) {
        walker.next();

        if(walker.fileInfo().completeSuffix() == "qml") {
            loadQmlFile(walker.filePath());
        }
    }

    foreach (ComponentInfo ci, g_componentInfos.values()) {
        QString name = ci.name;
        QString ciInfo = QString("file: %1:%2:%3").arg(ci.file).arg(ci.line).arg(ci.column);
        ciInfo += ", type: " + ci.type;
        if (!ci.objectName.isEmpty()) {
            ciInfo += ", named: " + ci.objectName;
        } else {
            ciInfo += ", unnamed";
        }
        qDebug("%s (%s)", name.toStdString().c_str(), ciInfo.toStdString().c_str());
    }
}
