#ifndef LASERVERLATENCYTHREAD_H
#define LASERVERLATENCYTHREAD_H

#include <QObject>

class LaNetwork;
class LaServerTableModel;
class QJsonObject;

class LaServerLatencyThread : public QObject
{
    Q_OBJECT
public:
    explicit LaServerLatencyThread(LaServerTableModel *model, QJsonObject *latencyTestsJson, QObject *parent = 0);
    
signals:
    void finished();

    void showLogMessage(QString msg);
    void showStatusBarLoadMessage(bool show, QString msg = QString());
    
public slots:
    void process();

private:
    LaNetwork *mLaNetwork;
    LaServerTableModel *mLaTableModel;
    QJsonObject *mLatencyTestsJson;
};

#endif // LASERVERLATENCYTHREAD_H
