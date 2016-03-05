#include <Engine/LaDataIO.h>
#include <Model/ModelItem/LaLoginItem.h>
#include <Model/ModelItem/LaServerItem.h>
#include <Util/LaCypher.h>

#include <QSettings>
#include <QDir>
#include <QApplication>
#include <QStringList>
#include <QDebug>

const QString FILE_AUTH = QString("auth.dat");
const QString AF_GROUP_AUTH = QString("Auth");
const QString AF_KEY_USER = QString("user");
const QString AF_KEY_PASS = QString("pass");
const QString AF_KEY_SAVE = QString("save");
const QString AF_KEY_AUTO = QString("auto");

const QString FILE_SERVER = QString("serverlist.dat");
const QString SLF_GROUP_VERSION = QString("Version");
const QString SLF_GROUP_PORT = QString("Port");
const QString SLF_GROUP_ROOT = QString("Root");
const QString SLF_KEY_ADDR = QString("addr");
const QString SLF_KEY_ADDR_BKP = QString("addrBkp");
const QString SLF_KEY_NAME = QString("name");
const QString SLF_KEY_SERVER_ADDR = QString("address");
const QString SLF_KEY_VERSION = QString("version");
const QString SLF_KEY_PORT = QString("port");

const QString FILE_LATENCY = QString("lat.dat");
const QString LAT_KEY_LAST_SENT = QString("sent_timestamp");

const QString TF_GROUP_IP = QString("ip");
const QString TF_GROUP_UDP = QString("udp");
const QString TF_GROUP_TUNNEL = QString("tunnel");
const QString TF_GROUP_RULES = QString("rules");
const QString TF_KEY_EXCLUDE_LOCAL = QString("excludelocal");
const QString TF_KEY_LOCAL_IP = QString("localip");
const QString TF_KEY_USE_NAT = QString("usenat");
const QString TF_KEY_LOCAL_HOST_NAME = QString("LocalHostName");
const QString TF_KEY_DEFAULT_FAKE_SERVER_IP = QString("defaultfakeserverip");
const QString TF_KEY_IS_GAME_GUARD = QString("IsGameGuard");
const QString TF_KEY_SOCKS_IS_SYS = QString("SocksIsSys");
const QString TF_KEY_INPUT_SOCKS_MESSAGE = QString("InputSocksMessage");
const QString TF_KEY_HOOK_NORMAL_SYS = QString("HookNormalSys");
const QString TF_KEY_HOOK_TIME = QString("HookTime");
const QString TF_KEY_RULE = QString("rule");

LaDataIO::LaDataIO(QObject *parent)
    : QObject(parent)
{
}

LaLoginItem LaDataIO::readLoginInfo() {
    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_AUTH;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    LaLoginItem mLaLoginItem;

    settings->beginGroup(AF_GROUP_AUTH);
    mLaLoginItem.setUserName( LaCypher::cypher(settings->value(AF_KEY_USER).toString()) );
    mLaLoginItem.setPassword( LaCypher::cypher(settings->value(AF_KEY_PASS).toString()) );
    mLaLoginItem.setSaveLogin( settings->value(AF_KEY_SAVE).toBool() );
    mLaLoginItem.setAutoLogin( settings->value(AF_KEY_AUTO).toBool() );
    settings->endGroup();

    return mLaLoginItem;
}

QString LaDataIO::readRootAddr() {
    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_SERVER;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    settings->beginGroup(SLF_GROUP_ROOT);
    QByteArray bRootAddr = settings->value(SLF_KEY_ADDR,
        QVariant("ZmFzdGVydHVubmVsLmNvbQ==")).toByteArray();
    QString addr = QString::fromUtf8(QByteArray::fromBase64(bRootAddr));
    settings->endGroup();

    return addr;
}

QString LaDataIO::readRootAddrBkp() {
    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_SERVER;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    settings->beginGroup(SLF_GROUP_ROOT);
    QByteArray bRootAddrBkp = settings->value(SLF_KEY_ADDR_BKP,
        QVariant("ZmFzdGVydHVubmVsLmNvbS5icg==")).toByteArray();
    QString addrBkp = QString::fromUtf8(QByteArray::fromBase64(bRootAddrBkp));
    settings->endGroup();

    return addrBkp;
}

bool LaDataIO::writeLoginInfo(LaLoginItem *mLaLoginItem) {
    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_AUTH;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    if(settings->status()) // [0-NoError] [1-AccessError] [2-FormatError]
        return false;

    settings->beginGroup(AF_GROUP_AUTH);
    settings->setValue(AF_KEY_USER, QVariant(LaCypher::cypher(mLaLoginItem->userName())));
    settings->setValue(AF_KEY_PASS, QVariant(LaCypher::cypher(mLaLoginItem->password())));
    settings->setValue(AF_KEY_SAVE, QVariant(mLaLoginItem->saveLogin()));
    settings->setValue(AF_KEY_AUTO, QVariant(mLaLoginItem->autoLogin()));
    settings->endGroup();

    return true;
}

bool LaDataIO::writeRootAddr(QString addr, QString addrBkp) {
    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_SERVER;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    if(settings->status()) // [0-NoError] [1-AccessError] [2-FormatError]
        return false;

    settings->beginGroup(SLF_GROUP_ROOT);
    settings->setValue(SLF_KEY_ADDR, QString::fromUtf8(
                           addr.toUtf8().toBase64()));
    settings->setValue(SLF_KEY_ADDR_BKP, QString::fromUtf8(
                           addrBkp.toUtf8().toBase64()));
    settings->endGroup();

    return true;
}

bool LaDataIO::writeTunnelInfo(QByteArray &data) {
    QString path = QString(qApp->applicationDirPath() + "/ss5/tunnel.ini");

    QFile tunnelFile(path);
    tunnelFile.open(QIODevice::ReadWrite);
    tunnelFile.write(data);
    tunnelFile.close();

    return true;
}

bool LaDataIO::writeLocalServerList(QByteArray &data) {
    QString tempPath = QString(qApp->applicationDirPath() + "/serverList.tmp");

    QFile tempFile(tempPath);
    tempFile.open(QIODevice::ReadWrite);
    tempFile.write(data);
    tempFile.close();

    QSettings *tempSettings = new QSettings(tempPath, QSettings::IniFormat);
    tempSettings->beginGroup(SLF_GROUP_VERSION);
    int newVersion = tempSettings->value(SLF_KEY_VERSION).toInt();
    tempSettings->endGroup();

    if( newVersion > LaDataIO::readLocalServerVersion() ) {
        QString path = QString(qApp->applicationDirPath() + "/serverList.dat");

        QFile serverListFile(path);
        if (serverListFile.exists()){
            serverListFile.remove();
        }
        serverListFile.open(QIODevice::ReadWrite);
        serverListFile.write(data);
        serverListFile.close();

//        QSettings *settings = new QSettings(path, QSettings::IniFormat);
//        settings->clear();


//        tempSettings->beginGroup(SLF_GROUP_VERSION);
//        int version = tempSettings->value(SLF_KEY_VERSION).toInt();
//        tempSettings->endGroup();

//        settings->beginGroup(SLF_GROUP_VERSION);
//        settings->setValue(SLF_KEY_VERSION, QVariant(version));
//        settings->endGroup();

//        tempSettings->beginGroup(SLF_GROUP_PORT);
//        int port = tempSettings->value(SLF_KEY_PORT).toInt();
//        tempSettings->endGroup();

//        settings->beginGroup(SLF_GROUP_PORT);
//        settings->setValue(SLF_KEY_PORT, QVariant(port));
//        settings->endGroup();

//        tempSettings->beginGroup(SLF_GROUP_ROOT);
//        QString rootAddr = QString(tempSettings->value(SLF_KEY_ADDR).toByteArray());
//        QString rootAddrBkp = tempSettings->value(SLF_KEY_ADDR_BKP).toString();
//        tempSettings->endGroup();

//        settings->beginGroup(SLF_GROUP_ROOT);
//        settings->setValue(SLF_KEY_ADDR, QVariant(rootAddr));
//        settings->setValue(SLF_KEY_ADDR_BKP, QVariant(rootAddrBkp));
//        settings->endGroup();

//        foreach (QString group, tempSettings->childGroups()) {
//            if(group.contains("SERVER_")) {
//                tempSettings->beginGroup(group);
//                QString name = tempSettings->value(SLF_KEY_NAME).toString();
//                QString addr = tempSettings->value(SLF_KEY_SERVER_ADDR).toString();
//                tempSettings->endGroup();

//                settings->beginGroup(group);
//                settings->setValue(SLF_KEY_NAME, QVariant(name));
//                settings->setValue(SLF_KEY_SERVER_ADDR, QVariant(addr));
//                settings->endGroup();
//            }
//        }

        tempFile.remove();
        return true;
    }
    tempFile.remove();
    return false;
}

bool LaDataIO::writeLatencyTimeStamp(qint64 lastTestTime)
{
    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_LATENCY;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    if(settings->status()) // [0-NoError] [1-AccessError] [2-FormatError]
        return false;

    settings->setValue(LAT_KEY_LAST_SENT, QVariant(lastTestTime));


    return true;
}

qint64 LaDataIO::readLatencyTimeStamp()
{
    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_LATENCY;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    qint64 addr = settings->value(LAT_KEY_LAST_SENT, QVariant(0)).toLongLong();

    return addr;
}

QList<LaServerItem*> * LaDataIO::readServerList() {
    QList<LaServerItem*> *serverList = new QList<LaServerItem*>;

    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_SERVER;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    foreach (const QString &group, settings->childGroups()) {
        if( group == SLF_GROUP_VERSION )
            continue;

        if( group == SLF_GROUP_PORT )
            continue;

        if( group == SLF_GROUP_ROOT )
            continue;

        settings->beginGroup(group);
        QByteArray bName = settings->value(SLF_KEY_NAME).toByteArray();
        QByteArray bAddress = settings->value(SLF_KEY_SERVER_ADDR).toByteArray();
        QString name = QString::fromUtf8( QByteArray::fromBase64(bName) );
        QString address = QString::fromUtf8( QByteArray::fromBase64(bAddress) );
        settings->endGroup();

        LaServerItem *serverItem = new LaServerItem(serverList->size(), name, address);

        serverList->append(serverItem);
    }

    return serverList;
}

int LaDataIO::readLocalServerVersion() {
    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_SERVER;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    settings->beginGroup(SLF_GROUP_VERSION);
    int version = settings->value(SLF_KEY_VERSION, 0).toInt();
    settings->endGroup();

    return version;
}

int LaDataIO::readServerPort() {
    QString path = qApp->applicationDirPath() + QDir::separator() + FILE_SERVER;
    QSettings *settings = new QSettings(path, QSettings::IniFormat);

    settings->beginGroup(SLF_GROUP_PORT);
    int port = settings->value(SLF_KEY_PORT, 0).toInt();
    settings->endGroup();

    return port;
}

