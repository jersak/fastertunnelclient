#ifndef LALOGTABLEMODEL_H
#define LALOGTABLEMODEL_H

#include <QStandardItemModel>

class LaLogTableModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit LaLogTableModel(QObject *parent = 0);
    void showLog(QString log);
};

#endif // LALOGTABLEMODEL_H
