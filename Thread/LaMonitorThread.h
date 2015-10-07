#ifndef LAMONITORTHREAD_H
#define LAMONITORTHREAD_H

#include <QObject>

class QTimer;
class LaRunTime;
class QUdpSocket;

class LaMonitorThread : public QObject
{
    Q_OBJECT
public:
    explicit LaMonitorThread(LaRunTime * runTime, QObject *parent = 0);
    void writeDatagram(const QByteArray &datagram);

private slots:
    // Monitor
    void onKillProcessTimeout();
    void onKeepAliveTimeout();
    void onFtcResponse();

private:
    bool _DEBUG_MONITOR_;
    bool _DEBUG_LOG_;

    void createConnections();
    void startCommunication();

    // Monitor controll vars
    int mMonitorFailtAttempts;

    QUdpSocket *mMonitorSocket;
    QTimer *mKeepAliveTimer;
    QTimer *mKillProcessTimer;

    LaRunTime * mRunTime;

};

#endif // LAMONITORTHREAD_H
