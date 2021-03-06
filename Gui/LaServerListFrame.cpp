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
#include <QDateTime>
#include <QComboBox>

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

void LaServerListFrame::enableViews(bool disable)
{
    mServerTableView->setEnabled(disable);
    mConnectButton->setEnabled(disable);
    mDiscconnectButton->setEnabled(disable);
    mCheckLatencyButton->setEnabled(disable);
    mUpdateServerButton->setEnabled(disable);
}

void LaServerListFrame::runUpdateServerThread() {
    mLaRunTime->showStatusBarLoadMessage(true, "Atualizando lista de servidores...");

    QThread *thread = new QThread(this);
    LaServerListThread *mLaServerListWorker = new LaServerListThread();
    mLaServerListWorker->moveToThread(thread);

    connect(mHookModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onHookModeChanged(int)));

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
    mCheckLatencyButton->setEnabled(false);

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
//    // TEMP START
//    if(mDNSLocal->isChecked())
//        mLaRunTime->DNSMode = 0;
//    else if(mDNSRemote->isChecked())
//        mLaRunTime->DNSMode = 2;
//    // TEMP END

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
    mCheckLatencyButton->setEnabled(true);
    mLaRunTime->showStatusBarLoadMessage(false);
    mLaRunTime->showLogMessage("Latência dos servidores atualizada.");

//    qint64 lastTimeStamp = LaDataIO::readLatencyTimeStamp();
//    qint64 currentTimeStamp = QDateTime::currentMSecsSinceEpoch();

    // Se time stamp do ultimo envio de testes for 0(nunca ter sido feito) ou feito a mais de meia hora
    // envia os resultados para o WS
//    if(lastTimeStamp == 0 || ((currentTimeStamp - lastTimeStamp) > 1800000)) {
//        sendTests();
//        LaDataIO::writeLatencyTimeStamp(currentTimeStamp);
//    }

    refreshTable();
}

void LaServerListFrame::onSS5ConnectionStateChange(bool connected) {
    mConnectButton->setVisible(!connected);
    mDiscconnectButton->setVisible(connected);
    mHookModeComboBox->setDisabled(connected);
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

void LaServerListFrame::onHookModeChanged(int mode)
{
    QString modeString = mHookModeComboBox->itemData(mode).toString();
    mLaRunTime->setHookMode(modeString);

    if(mode == 0 || mode == 1) // Se Driver ou Fast mode desabilita o LSP
        mLaRunTime->enableLSP(false);
    else mLaRunTime->enableLSP(true); // Habilita o LSP
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
//    tempLayout->addWidget(mDNSLocal);
//    tempLayout->addWidget(mDNSRemote);
    tempLayout->addWidget(mHookModeComboBox);
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
//    mDNSLocal = new QCheckBox("DNS Local", this);
//    mDNSRemote= new QCheckBox("DNS Remoto", this);

//    mDNSLocal->setAutoExclusive(true);
//    mDNSRemote->setAutoExclusive(true);

//    mDNSLocal->setChecked(true);
    // TEMP END

    /* 1 - Driver Mode
     * 5 - Fast mode
     * 6 - LSP Mode
     */
    mHookModeComboBox = new QComboBox(this);
    mHookModeComboBox->addItem("Driver Mode", QVariant("1"));
    mHookModeComboBox->addItem("Fast Mode", QVariant("5"));
    mHookModeComboBox->addItem("LSP Mode", QVariant("1"));

    mHookModeComboBox->setToolTip("Maneira como o cliente se acopla ao jogo.\n \
A maioria dos jogos deve funcionar como Driver Mode, porém pode ser necessário\n \
alterar o modo de acoplamento caso seu jogo não esteja sendo detectado.");

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
