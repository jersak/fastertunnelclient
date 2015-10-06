#ifndef LARRUNTIME_H
#define LARRUNTIME_H

#include <QString>
#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QSet>
#include <QList>

class LaNetwork;
class LaServerItem;
class QUdpSocket;
class QFile;

class LaProcessItem {
public:
    LaProcessItem() {
        pid=0;
        totalSent=0;
        totalReceived=0;
    }

    long pid;
    double totalSent;
    double totalReceived;
};

class LaRunTime : public QObject
{
    Q_OBJECT
public:
    LaRunTime(QObject *parent=0);

    //TEMP START
    int DNSMode;
    //TEMP END

    void changeLoginState(bool loggedIn);
    void setCurrentHash(QString hash) { mCurrentHash = hash; }
    void setCurrentUser(QString user) { mCurrentUser = user; }
    void setUsingTrialAccount(bool usingTrial) { mUsingTrialAccount = usingTrial; }
    void startNewLogFile();

    void writeLog(QString log);

    void showStatusBarLoadMessage(bool show, QString msg = QString());
    void showLogMessage(QString msg);

    QString currentHash() { return mCurrentHash; }
    QString currentUser() { return mCurrentUser; }
    bool loginState() { return mLoginState; }

    // SS5 engine
    void SS5StartEngine();
    bool SS5EngineIsRunning() { return mSS5EngineIsRunning; }
    bool SS5TunnelIsConnected() { return mSS5TunnelIsConnected; }
    bool isUsingTrialAccount() { return mUsingTrialAccount; }

signals:
    void SS5ReadyToConfig();
    void SS5ReadyToStartTunnel();
    void onLoginStateChange(bool state);
    void SS5ConnectStateChanged(bool connected);
    void onShowStatuBarLoadMessage(bool show, QString msg = QString());
    void onShowLogMessage(QString msg);
    void onShowSysTrayMsg(QString title, QString msg);
    void dataTransfer(double totalSent, double totalReceived);

public slots:
    void connectSS5(LaServerItem *server);
    void updateDataTransfer();
    void terminateSS5Engine();
    void disconnectSS5();

private slots:
    // FasterTunnel
    void onHashTimer();
    void onNoHashResponse();
    void compareHash(QString serverHash);
    void onTrialTimerTimeout();

    // Monitor
    void onKillProcessTimeout();
    void onKeepAliveTimeout();
    void onFtcResponse();
    void onClienteWithoutMonitor();

    // SS5
    void onSocketLogOutput();
    void SS5ConfigTunnel();
    void SS5StartTunnel();

private:
    bool _DEBUG_MONITOR_;
    bool _DEBUG_LOG_;

    void createConnections();

    // FasterTunnel
    void checkLicense();

    // Data Transfer
    void updateDataTransferList(long pId, double sentData, double receivedData);
    void clearDataTransferList();
    LaProcessItem *getTotalDataTransfer();

    // SS5 Engine
    void setSS5Runing(bool running) { mSS5EngineIsRunning = running; }
    void setSS5TunnelConnected(bool connected);
    void configSS5();

    // Monitor
    void killProcessIds();
    void storeProcessId(int pId);
    void clearProcessIds();

    QString magicNumber();

    LaNetwork *mLaNetwork;

    // Monitor controll vars
    int mMonitorFailtAttempts;

    QUdpSocket *mMonitorSocket;
    QTimer *mKeepAliveTimer;
    QTimer *mKillProcessTimer;
    QTimer *mKillClientWithoutMonitorTimer;

    QSet<int> mProcessIdList;

    // FasterTunnel controll vars
    LaServerItem *mCurrentServer;
    QTimer *mCheckHashTimer;
    QTimer *mTrialAccountTimer;

    int checkHashAtempts;
    bool mLoginState;
    bool mUsingTrialAccount;
    QString mCurrentHash;
    QString mCurrentUser;

    // SS5Engine controll vars
    QProcess mProcessEngine;
    QUdpSocket *mLogSocket;

    bool mSS5EngineIsRunning;
    bool mSS5TunnelIsConnected;

    // SS5 Tunnel Transfer Data
    QTimer *mDataTransferTime;
    QList<LaProcessItem*> mProcessListData;

    // Log
    QFile *mLogFile;
};

#endif // LARRUNTIME_H
