#ifndef LACLIENTCOMMTHREAD_H
#define LACLIENTCOMMTHREAD_H


#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QUdpSocket>
#include <QFile>
#include <QThread>


class LaClientCommThread : public QObject
{
    Q_OBJECT
private:
    int mMonitorFailtAttempts;
    QTimer* CheckMonitorProcessTimer;
public:
    LaClientCommThread(QObject *parent=0);
    void writeLog(QString log);
    QFile *mLogFile;
    void startConns();
public slots:
       void checkMonitorProcess();
};

#endif // LACLIENTCOMM_H
