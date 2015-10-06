#ifndef LAMAINWINDOW_H
#define LAMAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

class LaLoginFrame;
class LaMainFrame;
class LaLogFrame;
class LaRunTime;

class QLabel;
class QToolButton;

class QSystemTrayIcon;
class QMenu;

class LaMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit LaMainWindow(LaRunTime *runTime, QWidget *parent = 0);

public slots:
    void showStatusBarLoadMessage(bool show, QString msg = QString());
    void showSysTrayMessage(QString title, QString msg);
    void onTunnelConnectionChanged(bool connected);
    void onDataTransfer(double totalSent, double totalReceived);

private slots:
    void onSysTrayQuit();
    void trayIconClicked(QSystemTrayIcon::ActivationReason reason);

private:
    void createLayout();
    void createWidgets();
    void createSysTray();
    void createConnections();
    void createStatusBarWidgets();


    LaLoginFrame *mLaLoginFrame;
    LaMainFrame *mLaMainFrame;
    LaLogFrame *mLaLogFrame;

    // StatusBar
    QMovie *mLoadIcon;
    QLabel *mLoadIconLabel;
    QLabel *mMessageLabel;
    QLabel *mDataTransferLabel;

    // SysTray
    QSystemTrayIcon *mTrayIcon;
    QMenu *mTrayMenu;
    QAction *mShowWindowAction;
    QAction *mQuitAction;

    QToolButton *mCloseButton;

    LaRunTime *mLaRunTime;

    int mMouseX;
    int mMouseY;

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *e);
};

#endif // LAMAINWINDOW_H
