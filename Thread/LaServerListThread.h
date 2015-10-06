#ifndef LASERVERLISTTHREAD_H
#define LASERVERLISTTHREAD_H

#include <QObject>

class LaNetwork;

class LaServerListThread : public QObject
{
    Q_OBJECT
public:
    explicit LaServerListThread(QObject *parent = 0);
    
public slots:
    void process();

    void onServerListUpdated();
    void onTunnelConfigUpdated();

signals:
    void finished();

    void showLogMessage(QString msg);
    void showStatusBarLoadMessage(bool show, QString msg = QString());

private:
    LaNetwork *mLaNetwork;

    bool mServerListUpdateFinished;
    bool mTunnelConfigUpdateFinished;
};

#endif // LASERVERLISTTHREAD_H
