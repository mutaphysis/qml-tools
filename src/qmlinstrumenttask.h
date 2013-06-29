#ifndef QMLINSTRUMENTTASK_H
#define QMLINSTRUMENTTASK_H

#include <QObject>
#include <QString>

class JsInstrument;
struct Replacement;

class QmlInstrumentTask : public QObject
{
    Q_OBJECT

public:
    explicit QmlInstrumentTask(QObject *parent = 0);
    
    bool instrument(const QString &in, const QString &out);
    bool instrumentFolder(const QString &in, const QString &out);
    bool instrumentFile(const QString &in, const QString &out);
    QString instrumentFile(const QString &filename, bool &okay);
    QString instrumentQml(const QString &data, const QString &filename, bool &okay);
    QString instrumentJs(const QString &code, const QString &filename, bool &okay);
public slots:
    
private:
    QString rewrite(const QString &originalContents,
                    const QList<Replacement> &replacements);

    JsInstrument *m_instrumenter;
};

#endif // QMLINSTRUMENTTASK_H
