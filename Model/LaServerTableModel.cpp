#include "LaServerTableModel.h"
#include <Engine/LaDataIO.h>
#include <Model/ModelItem/LaServerItem.h>

#include <QList>
#include <QDebug>

LaServerTableModel::LaServerTableModel(QObject *parent)
    : QStandardItemModel(parent)
{
    mServerList = new QList<LaServerItem*>;
}

void LaServerTableModel::loadServerListFromFile() {
    mServerList = LaDataIO::readServerList();
}

void LaServerTableModel::refreshTable() {
    if( !mServerList->isEmpty() ) {
        clear();

        foreach (LaServerItem *serverItem, *mServerList) {
            QList<QStandardItem*> itemList;

            QStandardItem *statusItem = new QStandardItem("");
            QStandardItem *nameItem = new QStandardItem(serverItem->name());
            nameItem->setData(serverItem->id(), Qt::UserRole+1);
            nameItem->setData(serverItem->address(), Qt::UserRole+2);
            nameItem->setData(serverItem->name(), Qt::UserRole+3);
            QStandardItem *latencyItem = new QStandardItem("-");
            if(serverItem->latency() > 0)
                latencyItem->setText(
                            QString::number(serverItem->latency())+"ms");
            latencyItem->setData(serverItem->latency(), Qt::UserRole+1);

            statusItem->setEditable(false);
            nameItem->setEditable(false);
            latencyItem->setEditable(false);
            latencyItem->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

            int ping = serverItem->latency();
            if( ping == 0 ) {
                statusItem->setIcon(QIcon(":/ball_gray.png"));
            } else if( ping > 0 && ping <= 250 ) {
                statusItem->setIcon(QIcon(":/ball_green.png"));
            } else if( ping > 250 && ping <= 300) {
                statusItem->setIcon(QIcon(":/ball_orange.png"));
            } else statusItem->setIcon(QIcon(":/ball_red.png"));

            itemList.append(statusItem);
            itemList.append(nameItem);
            itemList.append(latencyItem);

            insertRow( rowCount(), itemList );
        }
    }

    setHorizontalHeaderItem(0, new QStandardItem());
    setHorizontalHeaderItem(1, new QStandardItem("Servidor"));
    setHorizontalHeaderItem(2, new QStandardItem("Latência"));
}

QVariant LaServerTableModel::data(const QModelIndex &index, int role) const {
    int col = index.column();

    if(role == Qt::TextAlignmentRole)
        if(col == 2)
            return Qt::AlignCenter;

    if(role == Qt::DecorationRole) {
        if(col == 0 ) {
            return itemFromIndex(index)->icon();
        }
    }
    else if(role == Qt::DisplayRole) {
        return itemFromIndex(index)->text();
    }

    return QVariant();
}
