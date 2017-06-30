#include "LaRunTime.h"
#include "LaNetwork.h"
#include "LaDataIO.h"
#include "Thread/LaClientCommThread.h"

#include <windows.h>
#include <stdio.h>

#include <Model/ModelItem/LaServerItem.h>
#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QUdpSocket>
#include <QFile>
#include <QThread>

#include <QDebug>

DWORD dwVolSerial;
BOOL bIsRetrieved;

const int HASH_VALIDATION_INTERVAL = 300000;
const int TRIAL_ACCOUNT_INTERVAL = 600000;

const int SS5_LOG_LOCAL_PORT = 50666;
const int MONITOR_WRITE_PORT = 50656;

const int MAX_MONITOR_ATTEMPTS = 3;
const int CHECK_MONITOR_INTERVAL = 3000;

const QString LICENSE_PATH = "HKEY_CURRENT_USER\\Software\\NetworkTunnel\\ss5capengine_fastertunnel";
const QString LICENSE_REG_NAME = "license";
const QString LICENSE_KEY = "V3YuL++s9hlcNjKOiMRBWy9aFFeiNr6SsAY8JS8KDNSXKpqNHOL9QigiUjaePBl0MDjSvzcREiMroVZfeY4aZphKocCTOzXdU2dZE0SRmw==";

const QString HWID_PATH = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Cryptography";
const QString HWID_REG_NAME = "MachineGuid";

LaRunTime::LaRunTime(QObject *parent)
    : QObject(parent)
{
    _DEBUG_LOG_ = true;
    _DEBUG_MONITOR_ = true;

    mLaNetwork = new LaNetwork(this);
    mLogSocket = new QUdpSocket(this);
    mLogSocket->bind(QHostAddress("127.0.0.1"), SS5_LOG_LOCAL_PORT);

    mMonitorSocket = new QUdpSocket(this);

    mLogFile = NULL;

    // Faster Tunnel
    mCheckHashTimer = new QTimer(this);
    mCheckHashTimer->setInterval(HASH_VALIDATION_INTERVAL);

    mTrialAccountTimer = new QTimer(this);

    mDataTransferTime = new QTimer(this);
    mDataTransferTime->setInterval(1000);

    // Enviar a lista de process id para o monitor via socket
    mMonitorCommunicationTimer = new QTimer(this);
    mMonitorCommunicationTimer->setInterval(1000);

    // Verificar na lista de processos do windows o FtcMonitor.exe
    mCheckMonitorProcessTimer = new QTimer(this);
    mCheckMonitorProcessTimer->setInterval(CHECK_MONITOR_INTERVAL);

    checkHashAtempts = 0;
    mLoginState = false;
    mSS5EngineIsRunning = false;
    mSS5TunnelIsConnected = false;
    mUsingTrialAccount = false;

    createConnections();
    checkLicense();
    checkHwId();

    mHookMode = "1";

    mMonitorCommunicationTimer->start();

    QProcess *p = new QProcess();
    QString ftcPath = "\"" + qApp->applicationDirPath() + "/FtcMonitor.exe\"";
    p->startDetached(ftcPath);

    mCheckMonitorProcessTimer->setSingleShot(true);
    mCheckMonitorProcessTimer->start(10000);

    //LaClientCommThread *commThread = new LaClientCommThread(this);

    //QThread *thread = new QThread(this);

    //commThread->moveToThread(thread);

    //thread->start();
}

void LaRunTime::createConnections() {

    // FasterTunnel
    connect(mCheckHashTimer, SIGNAL(timeout()), this, SLOT(onHashTimer()));
    connect(mLaNetwork, SIGNAL(hashResponse(QString)), this, SLOT(compareHash(QString)));
    connect(mLaNetwork, SIGNAL(noResponseFromHashServer()), this, SLOT(onNoHashResponse()));

    connect(this, SIGNAL(SS5ReadyToConfig()), this, SLOT(SS5ConfigTunnel()));
    connect(this, SIGNAL(SS5ReadyToStartTunnel()), this, SLOT(SS5StartTunnel()));

    connect(mDataTransferTime, SIGNAL(timeout()), this, SLOT(updateDataTransfer()));

    // Send process ids to monitor
    connect(mMonitorCommunicationTimer, SIGNAL(timeout()), this, SLOT(sendProccessIds()));

    connect(mCheckMonitorProcessTimer, SIGNAL(timeout()), this, SLOT(checkMonitorProcess()));

    // SS5 log response
    connect(mLogSocket, SIGNAL(readyRead()), this, SLOT(onSocketLogOutput()));
}

// Notifica via signals os objectos conectados
// se houve alteração no status da autenticação
void LaRunTime::changeLoginState(bool loggedIn) {
    mLoginState = loggedIn;
    emit onLoginStateChange(loggedIn);

    loggedIn ? mCheckHashTimer->start() :
               mCheckHashTimer->stop();

    if( !loggedIn && SS5EngineIsRunning() ) {
        if( SS5TunnelIsConnected() )
            disconnectSS5();
        terminateSS5Engine();
        killProcessIds();
    }
}

void LaRunTime::startNewLogFile() {

    QString fileName = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss") +
            ".log";
    QString path = qApp->applicationDirPath() + QDir::separator() +
            "logs" + QDir::separator();

    if(mLogFile && mLogFile->isOpen())
        mLogFile->close();

    QDir().mkdir(path);
    mLogFile = new QFile(path+fileName);
    mLogFile->open(QFile::WriteOnly | QIODevice::Text);
}

void LaRunTime::writeLog(QString log) {
    if(!mLogFile)
        startNewLogFile();

    QString time = QTime::currentTime().toString("hh:mm:ss: ");
    mLogFile->write(time.toUtf8() + log.toUtf8());
    mLogFile->write("\n");
    mLogFile->flush();
}

// TODO: fazer ligação direta
void LaRunTime::showStatusBarLoadMessage(bool show, QString msg) {
    emit onShowStatuBarLoadMessage(show, msg);
}

// TODO: fazer ligação direta
void LaRunTime::showLogMessage(QString msg) {
    emit onShowLogMessage(msg);
}

/**
 * @brief LaRunTime::compareHash
 * @param serverHash
 */
void LaRunTime::compareHash(QString serverHash) {
    qDebug() << "Server Hash: " << serverHash;
    qDebug() << "Current Hash: " << currentHash();

    if(serverHash != currentHash()) {
        qDebug() << "Hash MUDOU!: " << serverHash;
        changeLoginState(false);
        writeLog(" Desconectado. Conta usada em mais de um computador.");
        emit onShowLogMessage("Desconectado. " \
                              "Esta conta está sendo utilizada em mais de um computador.");
        emit onShowSysTrayMsg("Desconectado",
                              "Esta conta está sendo utilizada em mais de um computador.");
    }

    qDebug() << "Hash Ok!: " << serverHash;

    checkHashAtempts = 0; // Zera o contador de falhas de verificação
}

void LaRunTime::onTrialTimerTimeout() {
    disconnectSS5();
    terminateSS5Engine();
    killProcessIds();

    writeLog(" Desconectado. Tempo do trial acabou.");

    emit onShowLogMessage("Desconectado. " \
                          "O tempo utilização da conta teste expirou.");
    emit onShowSysTrayMsg("Desconectado",
                          "O tempo utilização da conta teste expirou..");
}

void LaRunTime::communicationLost() {

    writeLog("Desconectado. Não foi possivel se comunicar com o monitor.");
    disconnectSS5();
    terminateSS5Engine();

    killProcessIds();
    if(_DEBUG_MONITOR_) qDebug() << "Kill all process";
    qApp->quit();
}

void LaRunTime::killProcessIds() {
    foreach (int pid, mProcessIdList) {
        QProcess *p = new QProcess(this);
        QString cmd="taskkill";
        QStringList params;
        params << "/pid" << QString::number(pid) << "/f";
        p->start(cmd,params);
        if(!p->waitForFinished()) {
            delete p;
            return;
        }

        QString result = QString(p->readAllStandardOutput());
        if(_DEBUG_MONITOR_) qDebug() << "KillProcess: " << result;
    }

    clearProcessIds();
}

void LaRunTime::storeProcessId(int pId) {
    mProcessIdList.insert(pId);
    writeProcessIds();
}

void LaRunTime::sendProccessIds() {
    QString processList = QString("");
    foreach (int pId, mProcessIdList.toList()) {
        processList += QString::number(pId) + ",";
    }

    QByteArray b = QByteArray(processList.toStdString().c_str());
    mMonitorSocket->writeDatagram(b, QHostAddress("127.0.0.1"), MONITOR_WRITE_PORT);
}

void LaRunTime::checkMonitorProcess()
{

    qDebug() << "Entrei no coiso";

    QSharedMemory monitorsignature;
    monitorsignature.setKey("1234");

    qDebug() << "Criei o objeto de memoria e setei a key";

    if(monitorsignature.create(512,QSharedMemory::ReadWrite)) {
        qDebug() << "Desconectado. Não foi possivel encontrar o monitor.";
        writeLog("Desconectado. Não foi possivel encontrar o monitor.");
        killProcessIds();

        qApp->quit();
    } else {
        qDebug() << "Monitor está rodando.";
    }

    if(mCheckMonitorProcessTimer->isSingleShot()){
        mCheckMonitorProcessTimer->setSingleShot(false);
        mCheckMonitorProcessTimer->setInterval(CHECK_MONITOR_INTERVAL);
        mCheckMonitorProcessTimer->start();
    }

    /*
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::MergedChannels);
    process.start("wmic.exe process get description");

    //process.waitForStarted();
    process.waitForFinished(5);

    QByteArray list = process.readAll();
    QString processList = QString(list);
    if(processList.contains("FtcMonitor.exe")) {
        qDebug() << "Faster Tunnel Monitor is running";
        mMonitorFailtAttempts=0;
    }
    else {
        qDebug() << "Faster Tunnel Monitor is NOT running";
        mMonitorFailtAttempts++;

        writeLog("Failed to find monitor process ["
                 + QString::number(mMonitorFailtAttempts) + " Attempts] - " + QString(list));

        if(mMonitorFailtAttempts >= MAX_MONITOR_ATTEMPTS) {
            writeLog("Desconectado. Não foi possivel encontrar o monitor.");
            killProcessIds();

            qApp->quit();
        }
    }*/
}

void LaRunTime::clearProcessIds() {
    mProcessIdList.clear();
}

void LaRunTime::writeProcessIds()
{
    QString path = qApp->applicationDirPath() + QDir::separator();
    QString fileName = "pids.bin";

    QFile * pIdsFile = new QFile(path + fileName);
    QDir().mkdir(path);
    pIdsFile = new QFile(path+fileName);
    pIdsFile->open(QFile::WriteOnly | QIODevice::Text);

    QString pidsList = "";
    foreach (int pid, mProcessIdList) {
        pidsList += QString::number(pid) + ";";
    }

    pIdsFile->write(pidsList.toUtf8());
    pIdsFile->flush();
    pIdsFile->close();
}

/**
 * @brief LaRunTime::checkLicense
 * Check if the SS5 engine license is installed
 * if is not installed, install the license
 */
void LaRunTime::checkLicense() {
    QSettings *settings = new QSettings(LICENSE_PATH,QSettings::NativeFormat);

    QString installedLicense = settings->value(LICENSE_REG_NAME).toString();

    // Verifica se a key instalada é igual a key atual
    bool differentKey = (installedLicense.compare(LICENSE_KEY, Qt::CaseSensitive)==0) ?
            false : true;

    // Se não existe key instalada cria o registro, ou se não é a key atual, atualiza
    if( installedLicense.isEmpty() || differentKey )
        settings->setValue(LICENSE_REG_NAME, QVariant(LICENSE_KEY));

    qDebug() << "License: " << installedLicense;
}

QString LaRunTime::checkHwId() {

    bIsRetrieved = GetVolumeInformation(TEXT("C:\\"), NULL, NULL, &dwVolSerial, NULL, NULL, NULL, NULL);

    QString qstr;

    if (bIsRetrieved) {
       qstr = QString::number(dwVolSerial,16).toUpper();
       qDebug() << "HWID: " << qstr;
       return qstr;
    } else {
       qDebug() << "Could not get volume serial";
       return NULL;
    }

}

void LaRunTime::setSS5TunnelConnected(bool connected) {
    mSS5TunnelIsConnected = connected;
    emit SS5ConnectStateChanged(SS5TunnelIsConnected());
}

QString LaRunTime::magicNumber() {
    QDate d = QDateTime::currentDateTimeUtc().date();
    int magicnum = ((d.year()+15)*d.day() - (d.month()-1)*d.day()+d.day()*d.day()+112);
    return QString("magicnum:")+QString::number(magicnum);
}

void LaRunTime::onHashTimer() {
    //if(!isUsingTrialAccount())
    mLaNetwork->requestHash(currentUser());
}

void LaRunTime::onNoHashResponse() {
    checkHashAtempts++;

    if( checkHashAtempts >= 3 ) {
        writeLog(" Desconectado. Nao foi possivel verificar dados de autenticacao por 3 tentativas.");
        changeLoginState(false);
        onShowLogMessage("Desconectado. Não foi possível se comunicar com o " \
                         "servidor de verificação por 3 tentativas.");
        emit onShowSysTrayMsg("Desconectado", "Não foi possível se comunicar com o " \
                              "servidor de verificação por 3 tentativas.");
    }
}

/**
 * @brief LaRunTime::connectSS5
 * @param server
 * Receives a LaServerItem and store in mCurrentServer
 * Start SS5 connect process
 */

void LaRunTime::connectSS5(LaServerItem *server) {
    mCurrentServer = server;

//    if(isUsingTrialAccount()) {
//        mTrialAccountTimer->singleShot(TRIAL_ACCOUNT_INTERVAL, this, SLOT(onTrialTimerTimeout()));
//        mCheckHashTimer->stop();
//    } else if(mTrialAccountTimer->isActive()) {
//        mTrialAccountTimer->stop();
//    }

    // Se a engine não estiver rodando, inicia a engine;
    if( !SS5EngineIsRunning() ) {
        SS5StartEngine();
    } else { // Se etiver rodando emite o signal para o slot onSS5EngineStart()
        emit SS5ReadyToConfig();
    }
}

/**
 * @brief LaRunTime::SS5StartEngine
 * Starts SS5 engine "ss5capengine_fastertunnel.exe"
 */
void LaRunTime::SS5StartEngine() {
    showLogMessage("Iniciando engine...");

    QStringList params;
    params << "start"
           << QString::number(SS5_LOG_LOCAL_PORT)
           << "use_status"
              //           << "debug"
           << magicNumber();

    QString path = qApp->applicationDirPath() + QDir::separator()
            + "ss5" + QDir::separator() + "ss5capengine_fastertunnel.exe";

    mProcessEngine.startDetached(path, params);
}

/**
 * @brief LaRunTime::SS5ConfigTunnel
 * Configures SS5 via "/ss5/ss5capcmd.exe" with the respective params:
 * Ss5capcmd 1 3 0 MTg3LjQ1LjIwNi42 1080 0 bm8= dXNlcjE= cGFzc3dvcmQ=
 * - Ss5capcmd é o executavel que passa comandos para a engine
 * - 1 é o numero de sequencia do comando (contador incremental)
 * - 3 é o tipo do comando (nesse caso, configurar a engine)
 * - 0 é o modo de resolução de DNS (0: local, 1: local -> remoto, 2: remoto)
 * - MTg3LjQ1LjIwNi42 é o IP do servidor em B64
 * - 1080 é a porta onde o servidor está executando
 * - 0 é o tipo do servidor (socks5)
 * - bm8= significa que não é usado https
 * - dXNlcjE= login do server em B64
 * - cGFzc3dvcmQ= senha do server em B64
 */
void LaRunTime::SS5ConfigTunnel() {
    if( SS5TunnelIsConnected() )
        disconnectSS5();

    showLogMessage("Configurando tunel...");

    // Read server port from file
    QString port = QString::number(LaDataIO::readServerPort());

    QString userB64 = QString("dXNlcjE=");
    QString passB64 = QString("aGc2NzVKS0hzZA==");

    if(isUsingTrialAccount()) {
        userB64 = QString("trial").toUtf8().toBase64();
        passB64 = QString("kjlh376JHjhOLD").toUtf8().toBase64();
    }

    QStringList params;
    params << "1"
           << "3"
//           << QString::number(DNSMode)/*"0"*/
           << "0"
           << mCurrentServer->address().toUtf8().toBase64()
           << port
           << "0"
           << "bm8="
           << userB64
           << passB64;

    QString path = qApp->applicationDirPath() + "/ss5/ss5capcmd.exe";
    QProcess *p = new QProcess();
    p->start(path, params);
}

void LaRunTime::SS5StartTunnel() {
    showLogMessage("Iniciando tunnel...");

    writeLog("Selected hook mode " + mHookMode);

    QStringList params;
    params << "2" << mHookMode;

    QString path = qApp->applicationDirPath() + "/ss5/ss5capcmd.exe";

    qDebug() << "Start Tunnel params: " << params;
    writeLog("SS5StartTunnel params: 2 " + mHookMode);

    QProcess *p = new QProcess();
    p->start(path, params);

    mDataTransferTime->start(1000);
}

void LaRunTime::enableLSP(bool enable) {
    if(enable) writeLog("LSP habilitado");
    else writeLog("LSP desabilitado");

    QString p2 = "1";
    if(!enable) p2 = "0";

    QStringList params;
    params << "6" << p2;

    QString path = qApp->applicationDirPath() + "/ss5/ss5capcmd.exe";

    QProcess *p = new QProcess();
    p->start(path, params);
}

void LaRunTime::setHookMode(QString hookMode)
{
    mHookMode = hookMode;
}

void LaRunTime::disconnectSS5() {

    QStringList params;
    params << "3" << "2";

    QString path = qApp->applicationDirPath() + "/ss5/ss5capcmd.exe";

    QProcess *p = new QProcess();
    p->start(path, params);

    emit showLogMessage("Tunel desconectado.");
    emit onShowSysTrayMsg("Desconectado", "Tunel desconectado.");
    setSS5TunnelConnected(false);

    QString time = QTime::currentTime().toString("hh:mm:ss: ");
    mLogFile->write(time.toUtf8() + " disconnected by client");

    LaProcessItem *pItem = getTotalDataTransfer();
    writeLog( QString("Sent: " + QString::number(pItem->totalSent) + "kb" +
                      "| Received: " + QString::number(pItem->totalReceived) + "kb"));

    //killProcessIds();

    //clearProcessIds(); // Send command to monitor
    mDataTransferTime->stop(); // stop data transfer timer
    clearDataTransferList();
}

void LaRunTime::onDisconnectShorcutPressed() {
    disconnectThroughCPort();
    qDebug() << "onDisconnectShortcutPressed";
}

void LaRunTime::disconnectThroughCPort() {
    // Read server port from file
    QString port = QString::number(LaDataIO::readServerPort());

    QStringList params;
    params << "/close"
           << "*"
           << "*"
           << "*"
           << port;

    QString path = qApp->applicationDirPath() + "/ss5/cports.exe";
    QProcess *p = new QProcess();
    p->start(path, params);
}

void LaRunTime::terminateSS5Engine() {

    QStringList params;
    params << "4" << "4";

    QString path = qApp->applicationDirPath() + "/ss5/ss5capcmd.exe";

    QProcess *p = new QProcess();
    p->start(path, params);

    setSS5Runing(false);
}

void LaRunTime::updateDataTransferList(long pId, double sentData,
                                       double receivedData) {
    bool isNewPId = true;

    foreach (LaProcessItem *p, mProcessListData) {
        if( p->pid == pId ) {
            p->totalSent = sentData;
            p->totalReceived = receivedData;
            isNewPId = false;
        }
    }

    if(isNewPId) {
        LaProcessItem *p = new LaProcessItem();
        p->pid = pId;
        p->totalSent = sentData;
        p->totalReceived = receivedData;
        mProcessListData.append(p);
    }
}

void LaRunTime::clearDataTransferList() {
    mProcessListData.clear();
    emit dataTransfer(0,0);
}

LaProcessItem * LaRunTime::getTotalDataTransfer() {
    LaProcessItem *pTotal = new LaProcessItem();

    foreach (LaProcessItem *p, mProcessListData) {
        pTotal->totalSent += p->totalSent;
        pTotal->totalReceived += p->totalReceived;
    }

    return pTotal;
}

void LaRunTime::updateDataTransfer() {
    LaProcessItem *p = getTotalDataTransfer();
    emit dataTransfer(p->totalSent, p->totalReceived);
}

/// ///////////////////////////////////// ///
/// ///////// SS5 Sockey Output ///////// ///

// Resposta da conexão local entre cliente e ss5 engine;
/**
 * @brief LaRunTime::onSocketLogOutput
 * Receives the response from SS5 engine via mLogSocket
 * at port from var SS5_LOG_LOCAL_PORT
 */
void LaRunTime::onSocketLogOutput() {

    while (mLogSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mLogSocket->pendingDatagramSize());
        mLogSocket->readDatagram(datagram.data(), datagram.size());

        QString log = QString(datagram.data());

        if(!log.contains("status") ) {
            writeLog(log);
        }

        if(log.contains("status:")) {
            long double sent = log.section("|", 5, 5).toLongLong()/1024;
            long double received = log.section("|", 6, 6).toLongLong()/1024;

            updateDataTransferList(log.section("|", 1, 1).toLong(),
                                   sent, received);
        }

        if(log.contains("start ss5capengine success.")) {
            setSS5Runing(true);
            emit SS5ReadyToConfig();
        }

        if(log.contains("seq:1:success-setup")) {
            emit SS5ReadyToStartTunnel();
        }

        if(log.contains("seq:2:success-Tunnel")) {
            setSS5TunnelConnected(true);
            emit onShowLogMessage("Tunel conectado ao servidor " + mCurrentServer->name());
            emit onShowSysTrayMsg("Tunel conectado", "Tunel conectado ao servidor " +
                                  mCurrentServer->name() + ".");
        }

        if(log.contains("seq:3:success-Untunnel")) {
            setSS5TunnelConnected(false);
            emit onShowLogMessage("Tunel desconectado");
            emit onShowSysTrayMsg("Tunel desconectado", "Tunel desconectado.");
        }

        // get Process ID & Name
        if(log.contains("pid=")) {
            QStringList ignoredExe;
            ignoredExe << "taskkill.exe" << "FtcMonitor.exe" << "ss5capcmd.exe"
                       << "conhost.exe";

            bool ignore = false;
            foreach (QString exe, ignoredExe) {
                if(log.contains(exe)) {
                    ignore = true;
                }
            }

            qDebug() << log;

            if(!ignore) {
                QStringList splitedLog = log.split(",");

                // get Process Id
                QString pIdSring = splitedLog.at(1);
                int pId = pIdSring.mid(pIdSring.indexOf("=")+1, pIdSring.indexOf(" ")-4).toInt();
                storeProcessId(pId);

                // get Process Name
                QString processName = splitedLog.at(0);
                processName = processName.remove("log:AutoTunnel Sys ");
                processName = processName.remove(".exe");

                QStringList splitedProcessName = processName.split(" ");
                processName = "";

                foreach (QString pNamePart, splitedProcessName) {
                    processName += " " + pNamePart.mid(0,1).toUpper() +
                            pNamePart.mid(1, pNamePart.size()-1).toLower();
                }

                qDebug() << "Process Name: " << processName;

                if ((processName.contains("Chrome")) || (processName.contains("Plugin-container")) || (processName.contains("Iexplore")) ){
                }else{
                    emit onShowSysTrayMsg("Nova aplicação conectada",
                                      "A aplicação \"" + processName +
                                      "\" está conectada via FasterTunnel.");
                }

                emit onShowLogMessage("A aplicação \"" + processName +
                                      "\" está conectada via FasterTunnel.");

            }
        }

        if(_DEBUG_LOG_) qDebug() << "Log: " << QString(datagram.data());
    }
}
