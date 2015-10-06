#ifndef LASERVERTABLEMODEL_H
#define LASERVERTABLEMODEL_H

#include <Model/ModelItem/LaServerItem.h>

#include <QStandardItemModel>
#include <QList>
#include <QVariant>

class LaServerTableModel : public QStandardItemModel
{
public:
    LaServerTableModel(QObject *parent=0);
    QList<LaServerItem*> * serverList() { return mServerList; }
    void loadServerListFromFile();
    void refreshTable();

public slots:

private:
    QList<LaServerItem*> *mServerList;

    QVariant data(const QModelIndex &index, int role) const;
};

#endif // LASERVERTABLEMODEL_H
