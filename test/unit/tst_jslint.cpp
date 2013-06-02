#include <QString>
#include <QtTest>
#include <QCoreApplication>

#include <scriptcollector.h>
#include <jslintstep.h>

class JsLint : public QObject
{
    Q_OBJECT
    
public:
    JsLint() {}
    
private Q_SLOTS:
    void testScriptCollector();
    void test2();
};

void JsLint::testScriptCollector()
{
    QString filename =  "test/cases/ScriptCollector.qml";

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << filename << ":" << file.errorString();
    }
    QString code = QTextStream(&file).readAll();

    ScriptCollector collector;
    collector.parse(code, QUrl::fromLocalFile(filename), filename);    


    QList<ScriptCollector::Script> scripts = collector.scripts();

    QCOMPARE(scripts.length(), 11);

    QCOMPARE(scripts.at(0).name, QString("firstFunction"));
    QCOMPARE(scripts.at(1).name, QString("secondFunction"));
    QCOMPARE(scripts.at(2).name, QString("functionProp"));
    QCOMPARE(scripts.at(3).name, QString("bindingProp"));
    QCOMPARE(scripts.at(4).name, QString("ternaryProp"));
    QCOMPARE(scripts.at(5).name, QString("scriptProp"));
    QCOMPARE(scripts.at(6).name, QString("overideProp"));
    QCOMPARE(scripts.at(7).name, QString("onHandler"));
    QCOMPARE(scripts.at(8).name, QString("inlineFunction"));
    QCOMPARE(scripts.at(9).name, QString("inlineListFunction"));
    QCOMPARE(scripts.at(10).name, QString("childFunction"));
}

void JsLint::test2()
{
    JsLintStep lint;
    lint.setup("jslint/jslint.js");
    QJsonDocument results = lint.lint("var a = 4 - eval('6'); \n print('hello')\nx = 4 == 3 + a;\n");
    qDebug() << results.toJson();
}

QTEST_MAIN(JsLint)
#include "tst_jslint.moc"
