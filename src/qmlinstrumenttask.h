#ifndef QMLINSTRUMENTTASK_H
#define QMLINSTRUMENTTASK_H

#include <QObject>

class JsInstrument;
struct Replacement;

class QmlInstrumentTask : public QObject
{
    Q_OBJECT
public:
    explicit QmlInstrumentTask(QObject *parent = 0);
    
    //instrumentFolder(const QString &foldername);
    void instrumentFile(const QString &filename);
signals:
    
public slots:
    
private:
    QString rewrite(const QString &originalContents,
                    const QList<Replacement> &replacements);


    JsInstrument *m_instrumenter;
};

#endif // QMLINSTRUMENTTASK_H
