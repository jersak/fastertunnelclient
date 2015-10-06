#ifndef LALOGFRAME_H
#define LALOGFRAME_H

#include "Model/LaLogTableModel.h"
#include <QFrame>

class QTableView;
class LaRunTime;

class LaLogFrame : public QFrame
{
    Q_OBJECT
public:
    explicit LaLogFrame(LaRunTime *runTime, QWidget *parent = 0);

signals:

public slots:
    void showLog(QString log);

private:
    void createWidgets();
    void createLayout();
    void createConnections();

    QTableView *mLogTable;

    LaLogTableModel mLogModel;

    LaRunTime *mLaRunTime;
};

#endif // LALOGFRAME_H
