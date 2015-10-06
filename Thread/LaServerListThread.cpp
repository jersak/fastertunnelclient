#include "LaServerListThread.h"

#include <Engine/LaNetwork.h>
#include <Engine/LaDataIO.h>

LaServerListThread::LaServerListThread(QObject *parent) :
    QObject(parent)
{
    mServerListUpdateFinished = false;
    mTunnelConfigUpdateFinished = false;

    mLaNetwork = new LaNetwork(this);

    connect(mLaNetwork, SIGNAL(writeServerListFinished()),
            this, SLOT(onServerListUpdated()));

    connect(mLaNetwork, SIGNAL(writeTunnelConfigFinished()),
            this, SLOT(onTunnelConfigUpdated()));

    connect(mLaNetwork, SIGNAL(showLogMessage(QString)),
            this, SIGNAL(showLogMessage(QString)));

    connect(mLaNetwork, SIGNAL(showStatusBarLoadMessage(bool,QString)),
            this, SIGNAL(showStatusBarLoadMessage(bool,QString)));
}

void LaServerListThread::process() {
    mLaNetwork->requestServerList();
//    mLaNetwork->requestTunnelConfig();
}

void LaServerListThread::onServerListUpdated() {
    mServerListUpdateFinished=true;

    if(mServerListUpdateFinished && mTunnelConfigUpdateFinished)
        emit finished();
}

void LaServerListThread::onTunnelConfigUpdated() {
    mTunnelConfigUpdateFinished=true;

    if(mServerListUpdateFinished && mTunnelConfigUpdateFinished)
        emit finished();
}
