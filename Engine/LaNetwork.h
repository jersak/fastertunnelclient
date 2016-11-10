#ifndef LANETWORK_H
#define LANETWORK_H

#include <QString>
#include <QObject>
#include <QHostInfo>

class QNetworkReply;
class LaServerItem;
class QJsonObject;

class LaNetwork : public QObject
{
    Q_OBJECT
public:
    LaNetwork(QObject *parent);

    void tryLogin(QString username, QString password, QString hash, QString hwid);
    void requestServerList();
    void requestTunnelConfig();
    void requestHash(QString currentUser);
    void requestAccountExpirationDate(QString username);
    void sendLatencyTests(QJsonObject * jsonObject);
    int pingServer(QString address);

private slots:
    void onLoginReply(QNetworkReply *reply);
    void onServerListReply(QNetworkReply *reply);
    void onTunnelConfigReplay(QNetworkReply *reply);
    void onHashReply(QNetworkReply *reply);
    void onHashBackupHostReply(QNetworkReply *reply);
    void onAccountExpirationReply(QNetworkReply * reply);
    void onLatencyTestsReply(QNetworkReply *reply);

signals:
    void loginResponse(bool validLogin);
    void hashResponse(QString serverHash);
    void accountExpirationResponse(QString expirationDate);
    void writeServerListFinished();
    void writeTunnelConfigFinished();

    void noResponseFromHashServer();
    void noResponseFromLoginServer();

    void showLogMessage(QString msg);
    void showStatusBarLoadMessage(bool show, QString msg = QString());

private:
    void requestHashOnBackupHost();

    QString mCurrentUser;
};

#endif // LANETWORK_H
