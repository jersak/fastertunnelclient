#ifndef LADATAMODEL_H
#define LADATAMODEL_H

#include <QObject>
#include <QList>

class LaLoginItem;
class LaServerItem;
class LaTunnelItem;

class LaDataIO : public QObject
{
    Q_OBJECT
public:
    LaDataIO(QObject *parent=0);

    static LaLoginItem readLoginInfo();
    static QString readRootAddr();
    static QString readRootAddrBkp();
    static QList<LaServerItem *> *readServerList();
    static int readLocalServerVersion();
    static int readServerPort();

    static bool writeLoginInfo(LaLoginItem *mLaLoginItem);
    static bool writeRootAddr(QString addr, QString addrBkp);
    static bool writeTunnelInfo(QByteArray &data);
    static bool writeLocalServerList(QByteArray &data);

signals:
    void writeServerListFinished();

};

#endif // LADATAMODEL_H
