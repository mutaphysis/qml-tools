#include <QString>
#include <QtTest>

#include <scriptcollector.h>

class JsLint : public QObject
{
    Q_OBJECT
    
public:
    JsLint();
    
private Q_SLOTS:
    void testCase1();
};

JsLint::JsLint()
{
}

void JsLint::testCase1()
{
    QString filename = "/Users/loplop/Programmieren/qml-lint/test/cases/jslint/Test.qml";

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open" << filename << ":" << file.errorString();
    }
    QString code = QTextStream(&file).readAll();

    ScriptCollector collector;
    collector.parse(code, QUrl::fromLocalFile(filename), filename);    

    qDebug() << "\n" << collector.scripts();
}

QTEST_MAIN(JsLint)
#include "tst_jslint.moc"
