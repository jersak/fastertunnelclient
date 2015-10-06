#ifndef LATUNNELITEM_H
#define LATUNNELITEM_H

#include <QString>

class LaTunnelItem
{
public:
    LaTunnelItem();

    //IP
    QString mExcludeLocal;
    QString mLocalIP;

    //UDP
    QString mUseNAT;
    QString mLocalHostName;
    QString mDefaultFakeServerIP;

    //Tunnel
    QString mIsGameGuard;
    QString mSocksIsSys;
    QString mInputSocksMessage;
    QString mHookNormalSys;
    QString mHookTime;

    //Rules
    QString mRule;

};

#endif // LATUNNELITEM_H
