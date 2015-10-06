#include "LaLogTableModel.h"

#include <QDateTime>

#include <QDebug>

LaLogTableModel::LaLogTableModel(QObject *parent) :
    QStandardItemModel(parent)
{
    setHorizontalHeaderItem(0, new QStandardItem("Data-Hora"));
    setHorizontalHeaderItem(1, new QStandardItem("Evento"));
}

void LaLogTableModel::showLog(QString log) {
    QDateTime *mDateTime = new QDateTime();

    QString time = mDateTime->currentDateTime().toString("dd/MM/yyyy hh:mm:ss");

    QStandardItem *timeItem = new QStandardItem(time);
    QStandardItem *eventItem = new QStandardItem(log);

    QList<QStandardItem*> logItemList;
    logItemList.append(timeItem);
    logItemList.append(eventItem);

    insertRow(rowCount(), logItemList);
}

