#include "LaMainFrame.h"
#include "LaServerListFrame.h"
#include "LaLogFrame.h"
#include <LaStyleSheet.h>
#include <Engine/LaRunTime.h>

#include <QHBoxLayout>
#include <QWebView>
#include <QWebFrame>
#include <QtNetwork>

#include <QDebug>

LaMainFrame::LaMainFrame(LaRunTime *runTime, QWidget *parent) :
    QFrame(parent)
{
    mLaRunTime = runTime;

    setStyleSheet( LaMainStyleSheet );
    setObjectName("GlossyFrame");

    createWidgets();
    createLayout();
    createConnections();
}

void LaMainFrame::createWidgets() {
    mLaServerListFrame = new LaServerListFrame(mLaRunTime, this);
    mLaServerListFrame->setDisabled(true);
    mWebView = new QWebView(this);
    mWebView->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    mWebView->load(QUrl("http://www.fastertunnel.com/gui_feed.html"));
    mWebView->setFixedWidth(520);
}

void LaMainFrame::createLayout() {
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setContentsMargins(0,0,0,5);
    setLayout(l);

    l->addWidget(mLaServerListFrame);
    l->addWidget(mWebView);
    l->setStretch(0,0);
    l->setStretch(1,1);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

void LaMainFrame::createConnections() {
    connect(mLaRunTime, SIGNAL(onLoginStateChange(bool)),
            mLaServerListFrame, SLOT(setEnabled(bool)));
}
