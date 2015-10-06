#include "LaLogFrame.h"
#include <Engine/LaRunTime.h>

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>

LaLogFrame::LaLogFrame(LaRunTime *runTime, QWidget *parent) :
    QFrame(parent)
{
    mLaRunTime = runTime;

    setFixedHeight(120);

    createWidgets();
    createLayout();
    createConnections();
}

void LaLogFrame::showLog(QString log) {
    mLogModel.showLog(log);
    mLogTable->resizeColumnToContents(0);
    mLogTable->resizeRowsToContents();
    mLogTable->scrollToBottom();
}

void LaLogFrame::createWidgets() {
    mLogTable = new QTableView(this);
    mLogTable->setModel(&mLogModel);
    mLogTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

void LaLogFrame::createLayout() {
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    setLayout(l);

    l->addWidget(mLogTable);
}

void LaLogFrame::createConnections() {
    connect(mLaRunTime, SIGNAL(onShowLogMessage(QString)),
            this, SLOT(showLog(QString)));
}
