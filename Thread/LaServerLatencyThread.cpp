#include "LaServerLatencyThread.h"

#include <Engine/LaNetwork.h>
#include <Model/LaServerTableModel.h>
#include <QJsonObject>
#include <QJsonArray>

LaServerLatencyThread::LaServerLatencyThread(LaServerTableModel *model, QJsonObject *latencyTestsJson, QObject *parent) :
    QObject(parent)
{
    mLaNetwork = new LaNetwork(this);
    mLaTableModel = model;
    mLatencyTestsJson = latencyTestsJson;
}

void LaServerLatencyThread::process() {
    emit showStatusBarLoadMessage(true, "Verificando latência dos servidores...");

    QJsonArray jsonArray;

    foreach(LaServerItem *serverItem, *mLaTableModel->serverList()) {
        int fastestConn=0;

        for(int i=0; i < 3; i++) {
            int latency = mLaNetwork->pingServer(serverItem->address());

            if(!fastestConn) fastestConn = latency;
            if( latency < fastestConn && latency > 0 ) fastestConn = latency;
        }

        QJsonObject jsonObject;
        jsonObject.insert("Server", QJsonValue(serverItem->name()));
        jsonObject.insert("ping",QJsonValue(fastestConn));

        jsonArray.append(jsonObject);

        serverItem->setLatency( fastestConn );
    }

    if(mLatencyTestsJson->count() > 0) {
        foreach (QString key, mLatencyTestsJson->keys()) {
            mLatencyTestsJson->remove(key);
        }
    }

    mLatencyTestsJson->insert("Client_IP", "");
    mLatencyTestsJson->insert("Tests", jsonArray);

    emit showStatusBarLoadMessage(false);
    emit this->finished();
}
