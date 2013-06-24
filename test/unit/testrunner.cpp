#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include <scriptcollector.h>
#include <jslint.h>
#include <jsinstrument.h>
#include <qmlinstrumenttask.h>

class TestRunner : public QObject
{
    Q_OBJECT
    
public:
    TestRunner() {}
    
private Q_SLOTS:
    void testScriptCollector();
    void testLint();
    void testJsInstrument();
    void testQmlInstrumentTask();
};

void testScript(
        const ScriptCollector::Script &script,
        const QString &name,
        const quint16 &startLine,
        const quint16 &startColumn,
        const quint16 &endLine,
        const quint16 &endColumn)
{
    qDebug() << "found" << script;
    QCOMPARE(script.name, name);
    QCOMPARE(script.location.start.line, startLine);
    QCOMPARE(script.location.start.column, startColumn);
    QCOMPARE(script.location.end.line, endLine);
    QCOMPARE(script.location.end.column, endColumn);
}

void TestRunner::testScriptCollector()
{
    QString filename = "test/cases/ScriptCollector.qml";

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << filename << ":" << file.errorString();
    }
    QString code = QTextStream(&file).readAll();

    ScriptCollector collector;
    collector.parse(code, QUrl::fromLocalFile(filename), filename);    


    QList<ScriptCollector::Script> scripts = collector.scripts();

    QCOMPARE(scripts.length(), 13);
    QCOMPARE(collector.objectStartOffset(), (quint32)60);
    QVERIFY(collector.errors().isEmpty());

    testScript(scripts.at(0), "firstFunction", 12, 30, 14, 5);
    testScript(scripts.at(1), "secondFunction", 16, 31, 18, 5);
    testScript(scripts.at(2), "functionProp", 22, 32, 22, 46);
    testScript(scripts.at(3), "bindingProp", 23, 31, 23, 42);
    testScript(scripts.at(4), "ternaryProp", 24, 31, 24, 45);
    testScript(scripts.at(5), "scriptProp", 25, 30, 33, 5);
    testScript(scripts.at(6), "overideProp", 34, 18, 34, 27);
    testScript(scripts.at(7), "onHandler", 34, 40, 34, 41);
    testScript(scripts.at(8), "two", 35, 30, 35, 38);
    testScript(scripts.at(9), "inlineFunction", 37, 67, 37, 68);
    testScript(scripts.at(10), "inlineListFunction", 38, 82, 38, 83);
    testScript(scripts.at(11), "onSomeEventHandler", 40, 35, 41, 5);
    testScript(scripts.at(12), "childFunction", 45, 35, 47, 9);
}

void TestRunner::testLint()
{
    JsLint lint;
    QJsonDocument results = lint.lint("var a = 4 - eval('6'); \n print('hello')\nx = 4 == 3 + a;\n");

    //qDebug() << results.toJson();
    QVERIFY(results.toJson().length() > 0);
}

void TestRunner::testJsInstrument()
{
    JsInstrument instrument;
    JsInstrument::Instrumented instrumented = instrument.instrument("{\n"
                                           "    print(4);"
                                           "    if (true) {"
                                           "        var a = x > 3 ? 'a' : 'b'"
                                           "    }"
                                           "}", "unknown.js", 5, 10);
    QString code = instrumented.code;
    QString property = instrumented.property;

    //qDebug() << property << "\n" << code;
    QVERIFY(code.length() > 0);
    QVERIFY(property.length() > 0);
}

void TestRunner::testQmlInstrumentTask()
{
    QmlInstrumentTask instrumenter;
    instrumenter.instrumentFile("test/cases/Coverage2.qml");
}


QTEST_MAIN(TestRunner)
#include "testrunner.moc"
