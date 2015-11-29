#ifndef LASERVERLISTFRAME_H
#define LASERVERLISTFRAME_H

#include <Model/LaServerTableModel.h>

#include <QFrame>
#include <QJsonObject>

class QTableView;
class QCheckBox;
class QToolButton;
class QPushButton;
class LaRunTime;
class LaNetwork;
class QComboBox;

class LaServerListFrame : public QFrame
{
    Q_OBJECT
public:
    explicit LaServerListFrame(LaRunTime *runTime, QWidget *parent = 0);
    
signals:

public slots:
    void enableViews(bool disable);

private slots:
    void runUpdateServerThread();
    void runCheckLatencyThread();
    void onConnectServerButtonClicked();
    void onDisconnectServerButtonClicked();
    void onUpdateServerThreadFinished();
    void onCheckLatencyThreadFinished();
    void onSS5ConnectionStateChange(bool connected);
    void loadServersFromFile();
    void refreshTable();
    void sortColumn(int index);
    void onHookModeChanged(int mode);

private:
    void createLayout();
    void createWidgets();
    void createConnections();
    void sendTests();

    QTableView *mServerTableView;
    QPushButton *mConnectButton;
    QPushButton *mDiscconnectButton;
    QToolButton *mCheckLatencyButton;
    QToolButton *mUpdateServerButton;

    QCheckBox *mDNSLocal;
    QCheckBox *mDNSRemote;

    QComboBox *mHookModeComboBox;

    LaServerTableModel mServerModel;
    QJsonObject mLatenxyTestsJson;

    LaNetwork *mLaNetwork;
    LaRunTime *mLaRunTime;
};

#endif // LASERVERLISTFRAME_H
