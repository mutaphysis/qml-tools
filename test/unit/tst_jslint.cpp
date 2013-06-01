#include <QString>
#include <QtTest>

#include <scriptcollector.h>

class JsLint : public QObject
{
    Q_OBJECT
    
public:
    JsLint();
    
private Q_SLOTS:
    void testScriptCollector();
};

JsLint::JsLint()
{
}

void JsLint::testScriptCollector()
{
    QString filename = "/Users/loplop/Programmieren/qml-lint/test/cases/jslint/Test.qml";

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

QTEST_MAIN(JsLint)
#include "tst_jslint.moc"
