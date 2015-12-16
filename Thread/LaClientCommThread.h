#ifndef LACLIENTCOMMTHREAD_H
#define LACLIENTCOMMTHREAD_H

#include <QObject>
#include <QSharedMemory>

class QFile;
class QTimer;
class LaRunTime;

class LaClientCommThread : public QObject
{
    Q_OBJECT

public:
    LaClientCommThread(LaRunTime *runtime, QObject *parent=0);

public slots:
    void checkMonitorProcess();

private:
    void writeLog(QString log);

    QFile *mLogFile;
    int mMonitorFailtAttempts;
    QTimer* mCheckMonitorProcessTimer;
    QSharedMemory monitorsignature;

    LaRunTime *mRunTime;
};

#endif // LACLIENTCOMM_H
