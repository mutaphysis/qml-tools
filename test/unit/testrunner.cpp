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
        const ScriptCollector::Type &type,
        const quint16 &startLine,
        const quint16 &startColumn,
        const quint16 &endLine,
        const quint16 &endColumn)
{
    qDebug() << "found" << script;
    QCOMPARE(script.name, name);
    QCOMPARE(script.location.start.line, startLine);
    QCOMPARE(script.type, type);
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

    // find 4th opening curly bracket
    int index = 0;
    for (int i = 0; i < 4; ++i) {
        index = code.indexOf("{", index) + 1;
    }

    QCOMPARE(scripts.length(), 14);
    QCOMPARE(collector.objectStartOffset(), (quint32)index);
    QVERIFY(collector.errors().isEmpty());

    testScript(scripts.at(0), "firstFunction", ScriptCollector::Function, 12, 30, 14, 5);
    testScript(scripts.at(1), "secondFunction", ScriptCollector::Function, 16, 31, 18, 5);
    testScript(scripts.at(2), "functionCallProp", ScriptCollector::Property, 22, 36, 22, 50);
    testScript(scripts.at(3), "bindingProp", ScriptCollector::Property, 23, 31, 23, 42);
    testScript(scripts.at(4), "ternaryProp", ScriptCollector::Property, 24, 31, 24, 45);
    testScript(scripts.at(5), "scriptProp", ScriptCollector::Property, 25, 30, 33, 5);
    testScript(scripts.at(6), "overideProp", ScriptCollector::Property, 34, 18, 34, 27);
    testScript(scripts.at(7), "onHandler", ScriptCollector::Property, 34, 40, 34, 41);
    testScript(scripts.at(8), "two", ScriptCollector::Property, 35, 30, 35, 38);
    testScript(scripts.at(9), "jsFunctionProp", ScriptCollector::AnonymousFunctionProperty, 43, 34, 44, 5);
    testScript(scripts.at(10), "inlineFunction", ScriptCollector::Function, 46, 67, 46, 68);
    testScript(scripts.at(11), "inlineListFunction", ScriptCollector::Function, 47, 82, 47, 83);
    testScript(scripts.at(12), "onSomeEventHandler", ScriptCollector::Property, 49, 35, 50, 5);
    testScript(scripts.at(13), "childFunction", ScriptCollector::Function, 55, 1, 56, 9);
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
    bool okay1 = instrumenter.instrumentFile("test/cases/Coverage.qml", "test/cases/Rewrite.qml");
    bool okay2 = instrumenter.instrumentFile("test/cases/Coverage2.qml", "test/cases/Rewrite2.qml");
    bool okay3 = instrumenter.instrumentFile("test/cases/Coverage.js", "test/cases/Rewrite.js");
    bool okay4 = instrumenter.instrumentFile("test/cases/Coverage2.js", "test/cases/Rewrite2.js");

    QVERIFY(okay1);
    QVERIFY(okay2);
    QVERIFY(okay3);
    QVERIFY(okay4);

    //instrumenter.instrumentFolder("test/", "demo/");
}


QTEST_MAIN(TestRunner)
#include "testrunner.moc"
