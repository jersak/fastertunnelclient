#ifndef LAMAINFRAME_H
#define LAMAINFRAME_H

#include <QFrame>

class LaServerListFrame;
class QWebView;
class LaRunTime;

class LaMainFrame : public QFrame
{
    Q_OBJECT
public:
    explicit LaMainFrame(LaRunTime *runTime, QWidget *parent = 0);

private:
    void createWidgets();
    void createLayout();
    void createConnections();

    LaServerListFrame *mLaServerListFrame;
    QWebView *mWebView;

    LaRunTime *mLaRunTime;
};

#endif // LAMAINFRAME_H
