#include "LaMonitorThread.h"

#include <QTimer>
#include <QString>
#include <QApplication>
#include <QUdpSocket>
#include "Engine/LaRunTime.h"

const int MONITOR_LISTEN_PORT = 50657;
const int MONITOR_WRITE_PORT = 50656;

const int KEEP_ALIVE_TIMER_INTERVAL = 750;
const int KILL_PROCESS_TIMER_INTERVAL = 750;

const int MAX_MONITOR_ATTEMPTS = 2;

LaMonitorThread::LaMonitorThread(LaRunTime * runTime, QObject *parent)
    : QObject(parent)
{
    _DEBUG_LOG_ = false;
    _DEBUG_MONITOR_ = false;

    mRunTime = runTime;

    mMonitorFailtAttempts = 0;

    // Monitor
    mMonitorSocket = new QUdpSocket(this);
    mMonitorSocket->bind(QHostAddress("127.0.0.1"), MONITOR_LISTEN_PORT);

    mKeepAliveTimer = new QTimer(this);
    mKeepAliveTimer->setInterval(KEEP_ALIVE_TIMER_INTERVAL);

    mKillProcessTimer = new QTimer(this);
    mKillProcessTimer->setInterval(KILL_PROCESS_TIMER_INTERVAL);

    createConnections();
    startCommunication();
}

void LaMonitorThread::startCommunication() {
    mKeepAliveTimer->start();
    mKillProcessTimer->start();
}

void LaMonitorThread::createConnections() {

    // Monitor Response
    connect(mMonitorSocket, SIGNAL(readyRead()), this, SLOT(onFtcResponse()));
    connect(mKeepAliveTimer, SIGNAL(timeout()), this, SLOT(onKeepAliveTimeout()));
    connect(mKillProcessTimer, SIGNAL(timeout()), this, SLOT(onKillProcessTimeout()));
}

void LaMonitorThread::onKillProcessTimeout() {
    mMonitorFailtAttempts++;
    if(_DEBUG_MONITOR_) qDebug() << "Kill timeout Attempts: " << mMonitorFailtAttempts;

    if(mMonitorFailtAttempts > MAX_MONITOR_ATTEMPTS) {
        if(_DEBUG_MONITOR_) qDebug() << "Communication Lost";
        mRunTime->communicationLost();
    }
}

void LaMonitorThread::onKeepAliveTimeout() {
    if(_DEBUG_MONITOR_) qDebug() << "Client --> FTC";
    QString s = QString("ClientIsRunning");
    QByteArray b = QByteArray(s.toStdString().c_str());
    mMonitorSocket->writeDatagram(b, QHostAddress("127.0.0.1"), MONITOR_WRITE_PORT);
}

void LaMonitorThread::onFtcResponse() {
    QByteArray datagram;
    datagram.resize(mMonitorSocket->pendingDatagramSize());
    mMonitorSocket->readDatagram(datagram.data(), datagram.size());

    QString r = QString(datagram.data());
    if(_DEBUG_MONITOR_) qDebug() << "Cliente Received Data: " << r;

    if(r.contains("MonitorIsRunning")) {
//        if(_DEBUG_MONITOR_) qDebug() << "MonitorIsRunning";
        mMonitorFailtAttempts = 0;

        if(!mKillProcessTimer->isActive()) {
            mKillProcessTimer->start();
        }
    }
}

void LaMonitorThread::writeDatagram(const QByteArray &datagram) {
    mMonitorSocket->writeDatagram(datagram, QHostAddress("127.0.0.1"), MONITOR_WRITE_PORT);
}

