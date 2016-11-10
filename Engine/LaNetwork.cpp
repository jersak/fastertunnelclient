#include "LaNetwork.h"
#include <Model/ModelItem/LaServerItem.h>
#include <Model/ModelItem/LaTunnelItem.h>
#include <Engine/LaDataIO.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QSettings>
#include <QApplication>
#include <QProcess>
#include <QDateTime>
#include <QFile>
#include <QTimer>
#include <QTcpSocket>
#include <QJsonDocument>

#include <QDebug>

const QString ADDR_PREFIX = "http://";
const QString HASH_ADDR_SUFIX = "/hash.php";
const QString LOGIN_ADDR_SUFIX = "/gui_auth.php";
const QString ACCOUNT_EXPIRATION_ADDR_SUFIX = "/expira.php";
const QString TRIAL_ACCOUNT_ADDR_SUFIX = "/trial.php";
const QString CLIENT_SERVER_LIST_ADDR_SUFIX = "/conf/serverlist.ini";
const QString TUNNEL_FILE_ADDR_SUFIX = "/conf/tunnel.ini";
const QString LATENCY_TEST_WS_ADDR = "http://www.fastertunnel.com/parse/create_reading.php";//"http://ft.leftapps.com/parse/create_reading.php";

LaNetwork::LaNetwork(QObject *parent)
    : QObject(parent)
{
}

// Envia post request para o login
void LaNetwork:: tryLogin(QString username, QString password, QString hash) {
    QNetworkAccessManager *nwam = new QNetworkAccessManager;
    connect(nwam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onLoginReply(QNetworkReply*)));

    qDebug() << "root: " << LaDataIO::readRootAddr();

    QUrl url = QUrl(ADDR_PREFIX + LaDataIO::readRootAddr() + LOGIN_ADDR_SUFIX );

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QVariant("application/x-www-form-urlencoded"));

    QByteArray data;
    QUrlQuery params;

    username = username.replace("+", "%2B");
    username = username.replace(" ", "%20");
    password = password.replace("+", "%2B");
    password = password.replace(" ", "%20");

    params.addQueryItem("login", username);
    params.addQueryItem("password", password);
    params.addQueryItem("hash", hash);

    data.append(params.toString(QUrl::FullyEncoded));

    // Send backup data to server
    QNetworkAccessManager *nwam2 = new QNetworkAccessManager;

    QUrl urlBkp = ADDR_PREFIX + LaDataIO::readRootAddrBkp() + LOGIN_ADDR_SUFIX;
    QNetworkRequest request2(urlBkp);

    request2.setHeader(QNetworkRequest::ContentTypeHeader,
                       QVariant("application/x-www-form-urlencoded"));

    nwam->post(request,data);
    nwam2->post(request2,data);
}

// Requisita ao servidor a lista de servidores
void LaNetwork::requestServerList() {
    QNetworkAccessManager *nwam = new QNetworkAccessManager;
    connect(nwam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onServerListReply(QNetworkReply*)));

    QUrl url = QUrl(ADDR_PREFIX + LaDataIO::readRootAddr() + CLIENT_SERVER_LIST_ADDR_SUFIX);
    QNetworkRequest request(url);
    nwam->get(request);
}

// Requisita ao servidor os dados do arquivo tunnel.ini
void LaNetwork::requestTunnelConfig() {
    QNetworkAccessManager *nwam = new QNetworkAccessManager;
    connect(nwam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onTunnelConfigReplay(QNetworkReply*)));

    QUrl url = QUrl(ADDR_PREFIX + LaDataIO::readRootAddr() + TUNNEL_FILE_ADDR_SUFIX);
    QNetworkRequest request(url);
    nwam->get(request);
}

// Requisita ao servidor a hash armazenada
void LaNetwork::requestHash(QString currentUser) {
    mCurrentUser = currentUser;

    QNetworkAccessManager *nwam = new QNetworkAccessManager;
    connect(nwam, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onHashReply(QNetworkReply*)));

    QUrl url = QUrl(ADDR_PREFIX + LaDataIO::readRootAddr() + HASH_ADDR_SUFIX);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    mCurrentUser = mCurrentUser.replace("+", "%2B");
    mCurrentUser = mCurrentUser.replace(" ", "%20");

    QByteArray data;
    QUrlQuery params;

    params.addQueryItem("login", mCurrentUser);
    data.append(params.toString());

    nwam->post(request,data);
}

void LaNetwork::requestAccountExpirationDate(QString username) {

    QNetworkAccessManager *nwam = new QNetworkAccessManager;
    connect(nwam, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onAccountExpirationReply(QNetworkReply*)));

    QUrl url = QUrl(ADDR_PREFIX + LaDataIO::readRootAddr() + ACCOUNT_EXPIRATION_ADDR_SUFIX);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    username = username.replace("+", "%2B");
    username = username.replace(" ", "%20");

    QByteArray data;
    QUrlQuery params;

    qDebug() << "Login: " << username;
    params.addQueryItem("login", username);
    data.append(params.toString());

    nwam->post(request,data);
}

// Requisita ao servidor backup a hash caso o primeiro tenha falhado
void LaNetwork::requestHashOnBackupHost() {
    QNetworkAccessManager *nwam = new QNetworkAccessManager;
    connect(nwam, SIGNAL(finished(QNetworkReply*)), this,
            SLOT(onHashBackupHostReply(QNetworkReply*)));

    QUrl url = QUrl(ADDR_PREFIX + LaDataIO::readRootAddrBkp() + HASH_ADDR_SUFIX);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QByteArray data;
    QUrlQuery params;

    params.addQueryItem("login", mCurrentUser);
    data.append(params.toString());

    nwam->post(request,data);
}

// Verifica a latencia do endereço e retorna como int
int LaNetwork::pingServer(QString address) {

    QTime t;
    t.start();

    QTcpSocket *socket = new QTcpSocket();
    socket->connectToHost(address, 51234);
     if (socket->waitForConnected(1000)) {
         socket->close();
         return t.elapsed();
     }
     else return 0;

//    QProcess *cmd = new QProcess(0);
//    QString ping="ping";
//    QStringList params;
//    params << "-n" << "1" << address;
//    cmd->start(ping,params);
//    if(!cmd->waitForFinished()) {
//        delete cmd;
//        return 0;
//    }

//    QString result = QString(cmd->readAllStandardOutput());
//    QStringList resultList = result.split(" ");
//    foreach (QString s, resultList) {
//        if(s.contains("ms")) {
//            QString ping;
//            foreach ( QChar c, s ) {
//                if( c.isNumber() )
//                    ping.append(c);
//            }
//            delete cmd;
//            return ping.toInt();
//        }
//    }

//    delete cmd;
//    return 0;
}

// Recebe a resposta do login, trata e envia um signal
// para a LaLoginFrame
void LaNetwork::onLoginReply(QNetworkReply *reply) {
    QString result = QString(reply->readLine());
    qDebug() << "Login reply: " << result;

    if( result.isEmpty() ) {
        emit noResponseFromLoginServer();
        return;
    }

    if( result.toInt() == 1 ) {
        loginResponse(true);
    }
    else
        loginResponse(false);
}

// Recebe a resposta do servidor com a lista de servidores
void LaNetwork::onServerListReply(QNetworkReply *reply) {

    QByteArray b = reply->readAll();

    if( LaDataIO::writeLocalServerList(b) ) {
        emit writeServerListFinished();
        emit showLogMessage("Lista de servidores atualizada.");
        requestTunnelConfig();
    } else {
        emit showStatusBarLoadMessage(false);
        emit showLogMessage("Não existem novos servidores disponíveis.");
        emit writeServerListFinished();
        emit writeTunnelConfigFinished();
    }
}

void LaNetwork::onTunnelConfigReplay(QNetworkReply *reply) {
    QByteArray b = reply->readAll();

    if( LaDataIO::writeTunnelInfo(b) ) {
        emit writeTunnelConfigFinished();
    }
}

// Retorna a hash armazenada no servidor
void LaNetwork::onHashReply(QNetworkReply *reply) {
    QString serverHash = QString(reply->readLine());

    qDebug() << "main hash" << serverHash;

    if(serverHash.isEmpty() || serverHash.toInt() == 0) { // Se não conseguir resposta do servidor
        qDebug() << "main hash server failed";
        requestHashOnBackupHost(); // Requisita hash do servidor backup
    }
    else emit hashResponse(serverHash); // Se obteve resposta emit signal
}

void LaNetwork::onHashBackupHostReply(QNetworkReply *reply) {
    QString serverHash = QString(reply->readLine());

    qDebug() << "backup hash" << serverHash;

    if(serverHash.isEmpty() || serverHash.toInt() == 0) { // Se não conseguir resposta do servidor
        qDebug() << "backup hash server failed";
        emit noResponseFromHashServer(); // Avisa que não obteve resposta de nenhum servidor
    }
    else emit hashResponse(serverHash);
}

// Retorna a data de expiração da conta deste usuário
void LaNetwork::onAccountExpirationReply(QNetworkReply *reply) {

    QString expirationDate = QString(reply->readAll());

    QDateTime date = QDateTime().fromString(expirationDate, "yyyy-MM-dd");

    accountExpirationResponse("Sua conta expira em: " + date.toString("dd/MM/yyyy"));

    qDebug() << "Expiration Date: " << expirationDate;
}

void LaNetwork::sendLatencyTests(QJsonObject *jsonObject) {
    QNetworkAccessManager *nwam = new QNetworkAccessManager;

    connect(nwam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onLatencyTestsReply(QNetworkReply*)));

    QUrl url = QUrl(LATENCY_TEST_WS_ADDR);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QVariant("application/x-www-form-urlencoded"));

    QJsonDocument document(*jsonObject);

    QString strJson(document.toJson(QJsonDocument::Compact));

    QByteArray data;
    QUrlQuery params;

//    QString jsonData = QString::fromUtf8(document.toJson());

//    jsonData.replace("\n", "");

    qDebug() << "JsonDataString: " << strJson;//jsonData;

    params.addQueryItem("json", strJson);

    data.append(params.toString(QUrl::FullyEncoded));

    nwam->post(request, data);
}

void LaNetwork::onLatencyTestsReply(QNetworkReply *reply) {

    QString latencyReply = QString(reply->readAll());

    qDebug() << "LatencyTest Reply: " << latencyReply;

}
