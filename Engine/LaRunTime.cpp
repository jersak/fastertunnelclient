#include "LaRunTime.h"
#include "LaNetwork.h"
#include "LaDataIO.h"

#include <Model/ModelItem/LaServerItem.h>
#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QUdpSocket>
#include <QFile>
#include <Thread/LaMonitorThread.h>
#include <QThread>

#include <QDebug>

const int HASH_VALIDATION_INTERVAL = 600000;
const int TRIAL_ACCOUNT_INTERVAL = 600000;

const int SS5_LOG_LOCAL_PORT = 50666;

const QString LICENSE_PATH = "HKEY_CURRENT_USER\\Software\\NetworkTunnel\\ss5capengine_fastertunnel";
const QString LICENSE_REG_NAME = "license";
const QString LICENSE_KEY = "Od/eXo+U1iUY14xDHBNmDzmQg2OmppMDTiWL5hN1W6/FPhJFBBWdn+HH" \
        "1aCJZV5sQIdZcYVGXdRm33CDjGD0Kl0VRp9K6652yRE7NJufOQ==";

LaRunTime::LaRunTime(QObject *parent)
    : QObject(parent)
{
    _DEBUG_LOG_ = false;
    _DEBUG_MONITOR_ = false;

    mLaNetwork = new LaNetwork(this);
    mLogSocket = new QUdpSocket(this);
    mLogSocket->bind(QHostAddress("127.0.0.1"), SS5_LOG_LOCAL_PORT);

    mLogFile = NULL;

    // Faster Tunnel
    mCheckHashTimer = new QTimer(this);
    mCheckHashTimer->setInterval(HASH_VALIDATION_INTERVAL);

    mTrialAccountTimer = new QTimer(this);

    mDataTransferTime = new QTimer(this);
    mDataTransferTime->setInterval(1000);

    checkHashAtempts = 0;
    mLoginState = false;
    mSS5EngineIsRunning = false;
    mSS5TunnelIsConnected = false;
    mUsingTrialAccount = false;

    createConnections();
    checkLicense();

    QThread *thread = new QThread(this);

    mMonitorThread = new LaMonitorThread(this);
    mMonitorThread->moveToThread(thread);
    thread->start();

    QProcess *p = new QProcess();
    QString ftcPath = "\"" + qApp->applicationDirPath() + "/FtcMonitor.exe\"";
    p->startDetached(ftcPath);
}

void LaRunTime::createConnections() {

    // FasterTunnel
    connect(mCheckHashTimer, SIGNAL(timeout()), this, SLOT(onHashTimer()));
    connect(mLaNetwork, SIGNAL(hashResponse(QString)), this, SLOT(compareHash(QString)));
    connect(mLaNetwork, SIGNAL(noResponseFromHashServer()), this, SLOT(onNoHashResponse()));

    connect(this, SIGNAL(SS5ReadyToConfig()), this, SLOT(SS5ConfigTunnel()));
    connect(this, SIGNAL(SS5ReadyToStartTunnel()), this, SLOT(SS5StartTunnel()));

    connect(mDataTransferTime, SIGNAL(timeout()), this, SLOT(updateDataTransfer()));

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

    writeLog(" Desconectado. Não foi possivel se comunicar com o monitor.");
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

    mProcessIdList.clear();
}

void LaRunTime::storeProcessId(int pId) {
    mProcessIdList.insert(pId);

    // Send pid to monitor
    QString s = QString("process_id:%0").arg(pId);
    QByteArray b = QByteArray(s.toStdString().c_str());
    mMonitorThread->writeDatagram(b);
}

void LaRunTime::clearProcessIds() {

    // Send command to clear processId, used on disconnect tunnel
    QString s = QString("clearPidList");
    QByteArray b = QByteArray(s.toStdString().c_str());
    mMonitorThread->writeDatagram(b);
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
    if(!isUsingTrialAccount())
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

    if(isUsingTrialAccount()) {
        mTrialAccountTimer->singleShot(TRIAL_ACCOUNT_INTERVAL, this, SLOT(onTrialTimerTimeout()));
        mCheckHashTimer->stop();
    } else if(mTrialAccountTimer->isActive()) {
        mTrialAccountTimer->stop();
    }

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
        passB64 = QString("trial").toUtf8().toBase64();
    }

    QStringList params;
    params << "1"
           << "3"
           << QString::number(DNSMode)/*"0"*/
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

    QStringList params;
    params << "2" << "1";

    QString path = qApp->applicationDirPath() + "/ss5/ss5capcmd.exe";

    QProcess *p = new QProcess();
    p->start(path, params);

    mDataTransferTime->start(1000);
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

    killProcessIds();

    clearProcessIds(); // Send command to monitor
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
