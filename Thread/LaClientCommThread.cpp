#include "LaClientCommThread.h"

#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QFile>
#include <QSharedMemory>

#include <Engine/LaRunTime.h>

#include <QDebug>

const int MAX_MONITOR_ATTEMPTS = 3;
const int CHECK_MONITOR_INTERVAL = 5000;

LaClientCommThread::LaClientCommThread(LaRunTime *runTime, QObject *parent)
    : QObject(parent)
{
    mRunTime = runTime;

    mCheckMonitorProcessTimer = new QTimer(parent);
    mCheckMonitorProcessTimer->setInterval(1000);

    connect(mCheckMonitorProcessTimer, SIGNAL(timeout()), this, SLOT(checkMonitorProcess()));

    mCheckMonitorProcessTimer->start(10000);

    mLogFile = NULL;
}

void LaClientCommThread::writeLog(QString log) {
    if(!mLogFile){
        QString fileName = QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss") +
                "_thread.log";
        QString path = qApp->applicationDirPath() + QDir::separator() +
                "logs" + QDir::separator();

        if(mLogFile && mLogFile->isOpen())
            mLogFile->close();

        QDir().mkdir(path);
        mLogFile = new QFile(path+fileName);
        mLogFile->open(QFile::WriteOnly | QIODevice::Text);
    }

    QString time = QTime::currentTime().toString("hh:mm:ss: ");
    mLogFile->write(time.toUtf8() + log.toUtf8());
    mLogFile->write("\n");
    mLogFile->flush();
}

void LaClientCommThread::checkMonitorProcess()
{
    qDebug() << "Entrei no coiso";

    QSharedMemory monitorsignature("61BB201D-3569-453e-9144-");
    if(monitorsignature.create(512,QSharedMemory::ReadWrite)==true) {
        qDebug() << "Desconectado. Não foi possivel encontrar o monitor.";
        writeLog("Desconectado. Não foi possivel encontrar o monitor.");
        mRunTime->killProcessIds();

        qApp->quit();
    } else {
        qDebug() << "Monitor está rodando.";
    }

/*
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::MergedChannels);
    process.start("wmic.exe process get description");

    process.waitForStarted(1000);
    process.waitForFinished(1000);

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
            mRunTime->killProcessIds();

            qApp->quit();
        }
    } */
}

