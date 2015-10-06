#ifndef LASERVERITEM_H
#define LASERVERITEM_H

#include <QString>

class LaServerItem
{
public:
    LaServerItem(int id, QString name, QString address);

    int id() { return mId; }
    QString name() { return mName; }
    QString address() { return mAddress; }
    int latency() { return mLatency; }
    bool isConnected() { return mConnected; }

    void setId( int id ) { mId = id; }
    void setName( QString name ) { mName = name; }
    void setAddress( QString address ) { mAddress = address; }
    void setLatency( int latency ) { mLatency = latency; }
    void setConnected( bool connected ) { mConnected = connected; }

private:
    int mId;
    QString mName;
    QString mAddress;
    int mLatency;
    bool mConnected;
};

#endif // LASERVERITEM_H
