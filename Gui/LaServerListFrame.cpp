#include "LaServerListFrame.h"
#include <Engine/LaRunTime.h>
#include <Engine/LaNetwork.h>
#include <Engine/LaDataIO.h>
#include <Thread/LaServerListThread.h>
#include <Thread/LaServerLatencyThread.h>
#include <LaStyleSheet.h>

#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QThread>
#include <QJsonDocument>

LaServerListFrame::LaServerListFrame(LaRunTime *runTime, QWidget *parent) :
    QFrame(parent)
{
    mLaRunTime = runTime;
    mLaNetwork = new LaNetwork(this);

    createWidgets();
    createLayout();
    createConnections();

    mServerTableView->setModel(&mServerModel);

    runUpdateServerThread();

    onSS5ConnectionStateChange(false);
}

void LaServerListFrame::runUpdateServerThread() {
    mLaRunTime->showStatusBarLoadMessage(true, "Atualizando lista de servidores...");

    QThread *thread = new QThread(this);
    LaServerListThread *mLaServerListWorker = new LaServerListThread();
    mLaServerListWorker->moveToThread(thread);

    connect(mLaServerListWorker, SIGNAL(showLogMessage(QString)),
            mLaRunTime, SIGNAL(onShowLogMessage(QString)));

    connect(mLaServerListWorker, SIGNAL(showStatusBarLoadMessage(bool,QString)),
            mLaRunTime, SIGNAL(onShowStatuBarLoadMessage(bool,QString)));

    connect(thread, SIGNAL(started()), mLaServerListWorker, SLOT(process()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(mLaServerListWorker, SIGNAL(finished()), this, SLOT(onUpdateServerThreadFinished()));
    connect(mLaServerListWorker, SIGNAL(finished()), mLaServerListWorker, SLOT(deleteLater()));
    thread->start();
}

void LaServerListFrame::runCheckLatencyThread() {
    QThread *thread = new QThread();

    LaServerLatencyThread *mLaServerLatencyWork = new LaServerLatencyThread(&mServerModel, &mLatenxyTestsJson);
    mLaServerLatencyWork->moveToThread(thread);

    connect(mLaServerLatencyWork, SIGNAL(showLogMessage(QString)),
            mLaRunTime, SIGNAL(onShowLogMessage(QString)));
    connect(mLaServerLatencyWork, SIGNAL(showStatusBarLoadMessage(bool,QString)),
            mLaRunTime, SIGNAL(onShowStatuBarLoadMessage(bool,QString)));

    connect(thread, SIGNAL(started()), mLaServerLatencyWork, SLOT(process()));
    connect(thread, SIGNAL(finished()), this, SLOT(onCheckLatencyThreadFinished()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
//    connect(mLaServerLatencyWork, SIGNAL(latencyTestJsonCreated(QJsonObject*)), this, SLOT(onLatencyTestJsonCreated(QJsonObject*)));
    connect(mLaServerLatencyWork, SIGNAL(finished()), thread, SIGNAL(finished()));
    connect(mLaServerLatencyWork, SIGNAL(finished()), mLaServerLatencyWork, SLOT(deleteLater()));
    thread->start();
}

void LaServerListFrame::onConnectServerButtonClicked() {
    QModelIndexList rowIndexList = mServerTableView->selectionModel()->selectedIndexes();

    if( rowIndexList.isEmpty() )
        return;

    QStandardItem *item = mServerModel.itemFromIndex(rowIndexList.at(1));
    if( !item )
        return;

    QString name = item->text();
    QString addr = item->data(Qt::UserRole+2).toString();
    int id = item->data(Qt::UserRole+1).toInt();

    LaServerItem *server = new LaServerItem(id, name, addr);
    // TEMP START
    if(mDNSLocal->isChecked())
        mLaRunTime->DNSMode = 0;
    else if(mDNSRemote->isChecked())
        mLaRunTime->DNSMode = 2;
    // TEMP END

    mLaRunTime->connectSS5(server);
}

void LaServerListFrame::onDisconnectServerButtonClicked() {
    mLaRunTime->writeLog("disconnected by User");
    mLaRunTime->disconnectSS5();
}

void LaServerListFrame::onUpdateServerThreadFinished() {
    mLaRunTime->showStatusBarLoadMessage(false);
    loadServersFromFile();
}

void LaServerListFrame::onCheckLatencyThreadFinished() {
    mLaRunTime->showStatusBarLoadMessage(false);
    mLaRunTime->showLogMessage("Latência dos servidores atualizada.");

    sendTests();

    refreshTable();
}

void LaServerListFrame::onSS5ConnectionStateChange(bool connected) {
    mConnectButton->setVisible(!connected);
    mDiscconnectButton->setVisible(connected);
}

void LaServerListFrame::loadServersFromFile() {
    mServerModel.loadServerListFromFile();
    refreshTable();
}

void LaServerListFrame::refreshTable() {
    mServerModel.refreshTable();
    mServerTableView->verticalHeader()->setVisible(false);
    mServerTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    mServerTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mServerTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    mServerTableView->resizeColumnToContents(0);
    mServerTableView->resizeRowsToContents();
    mServerTableView->sortByColumn(1, Qt::AscendingOrder);
}

void LaServerListFrame::sortColumn(int index) {
    if(index == 1) {
        mServerModel.setSortRole(Qt::UserRole+3);
    } else if(index == 2) {
        mServerModel.setSortRole(Qt::UserRole+1);
    }
    mServerTableView->sortByColumn(index);
}

void LaServerListFrame::createLayout() {
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    setLayout(l);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(mCheckLatencyButton, 0, Qt::AlignLeft|Qt::AlignVCenter);
    bottomLayout->addWidget(mUpdateServerButton, 0, Qt::AlignLeft|Qt::AlignVCenter);
    bottomLayout->addWidget(mConnectButton, 0, Qt::AlignRight|Qt::AlignVCenter);
    bottomLayout->addWidget(mDiscconnectButton, 0, Qt::AlignRight|Qt::AlignVCenter);
    bottomLayout->setContentsMargins(0,0,0,0);
    bottomLayout->setStretch(1,1);

    // TEMP START
    QHBoxLayout *tempLayout = new QHBoxLayout();
    tempLayout->addWidget(mDNSLocal);
    tempLayout->addWidget(mDNSRemote);
    tempLayout->setContentsMargins(15, 0, 15, 0);
    // TEMP END

    l->addWidget(mServerTableView);
    l->addLayout(bottomLayout);
    l->addLayout(tempLayout);
}

void LaServerListFrame::createWidgets() {
    mServerTableView = new QTableView();
    mCheckLatencyButton = new QToolButton(this);
    mCheckLatencyButton->setIcon(QIcon(":/refresh.png"));
    mCheckLatencyButton->setStyleSheet("background: transparent;");
    mCheckLatencyButton->setIconSize(QSize(24,24));
    mCheckLatencyButton->setToolTip("Verificar latência");
    mUpdateServerButton = new QToolButton(this);
    mUpdateServerButton->setIcon(QIcon(":/update.png"));
    mUpdateServerButton->setStyleSheet("background: transparent;");
    mUpdateServerButton->setIconSize(QSize(24,24));
    mUpdateServerButton->setToolTip("Atualizar lista de servidores");
    mConnectButton = new QPushButton("Conectar", this);
    mConnectButton->setStyleSheet(LaConnectButtonStyleSheet);
    mDiscconnectButton = new QPushButton("Desconectar", this);
    mDiscconnectButton->setStyleSheet(LaConnectButtonStyleSheet);

    // TEMP START
    mDNSLocal = new QCheckBox("DNS Local", this);
    mDNSRemote= new QCheckBox("DNS Remoto", this);

    mDNSLocal->setAutoExclusive(true);
    mDNSRemote->setAutoExclusive(true);

    mDNSLocal->setChecked(true);
    // TEMP END
}

void LaServerListFrame::createConnections() {
    connect(mUpdateServerButton, SIGNAL(clicked()),
            this, SLOT(runUpdateServerThread()));

    connect(mCheckLatencyButton, SIGNAL(clicked()),
            this, SLOT(runCheckLatencyThread()));

    connect(mConnectButton, SIGNAL(clicked()),
            this, SLOT(onConnectServerButtonClicked()));

    connect(mDiscconnectButton, SIGNAL(clicked()),
            this, SLOT(onDisconnectServerButtonClicked()));

    connect(mLaRunTime, SIGNAL(SS5ConnectStateChanged(bool)),
            this, SLOT(onSS5ConnectionStateChange(bool)));

    connect(mServerTableView->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(sortColumn(int)));
}

void LaServerListFrame::sendTests() {
    mLaNetwork->sendLatencyTests(&mLatenxyTestsJson);
}
