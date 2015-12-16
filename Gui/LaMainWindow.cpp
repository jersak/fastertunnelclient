#include "LaMainWindow.h"
#include "LaLoginFrame.h"
#include "LaMainFrame.h"
#include "LaLogFrame.h"
#include "LaStyleSheet.h"
#include <Engine/LaRunTime.h>

#include <Util/lacypher.h>

#include <QVBoxLayout>
#include <QStatusBar>
#include <QBitmap>
#include <QLabel>
#include <QToolButton>
#include <QMovie>
#include <QMenu>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>

#include <QDebug>

LaMainWindow::LaMainWindow(LaRunTime *runTime, QWidget *parent) :
    QMainWindow(parent)
{
    setFixedSize(800,600);

    mLaRunTime = runTime;

    createWidgets();
    createLayout();
    createSysTray();
    createStatusBarWidgets();
    createConnections();

    setStyleSheet(LaMainWindowStyleSheet);
    setObjectName("GlossyFrame");

    QMessageBox msgBoxAviso;
    msgBoxAviso.setText("AVISO: Este programa deve ser executado como administrador!\n(Se você executou como administrador, desconsidere esta mensagem.)");
    msgBoxAviso.exec();
}

void LaMainWindow::showStatusBarLoadMessage(bool show, QString msg) {
    show ? mLoadIcon->start() : mLoadIcon->stop();
    mMessageLabel->setText(msg);
    mLoadIconLabel->setVisible(show);
}

void LaMainWindow::showSysTrayMessage(QString title, QString msg) {
    mTrayIcon->showMessage(title,msg);
}

void LaMainWindow::onTunnelConnectionChanged(bool connected) {
    connected ? hide() : show();
}

void LaMainWindow::onDataTransfer(double totalSent, double totalReceived) {
    QString sentFormat;
    QString receivedFormat;

    if( totalSent > 1024 ) {
        sentFormat = QString::number(totalSent/1024, 'f', 2) + " Mb";
    } else sentFormat = QString::number(totalSent, 'f', 2) + " Kb";

    if( totalReceived > 1024 ) {
        receivedFormat = QString::number(totalReceived/1024, 'f', 2) + " Mb";
    } else receivedFormat = QString::number(totalReceived, 'f', 2) + " Kb";

    QString dataText =
            "Enviados: " + sentFormat + " | " +
            "Recebidos: " + receivedFormat;

    mDataTransferLabel->setText(dataText);
}

void LaMainWindow::onSysTrayQuit() {
    close();
    qApp->quit();
}

void LaMainWindow::closeEvent(QCloseEvent *e) {
    mLaRunTime->terminateSS5Engine();

    QMainWindow::closeEvent(e);
}

void LaMainWindow::trayIconClicked(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        break;
    case QSystemTrayIcon::DoubleClick:
        show();
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    default:
        ;
    }
}

void LaMainWindow::createWidgets() {
    mLaLoginFrame = new LaLoginFrame(mLaRunTime, this);
    mLaMainFrame = new LaMainFrame(mLaRunTime, this);
    mLaLogFrame = new LaLogFrame(mLaRunTime, this);

    mCloseButton = new QToolButton(this);
    mCloseButton->setIcon(QIcon(":/close.png"));
    mCloseButton->setStyleSheet("background: transparent;");
}

void LaMainWindow::createSysTray() {
    mShowWindowAction = new QAction(tr("Exibir"), this);
    mQuitAction = new QAction(tr("Sair"), this);

    mTrayMenu = new QMenu(this);
    mTrayMenu->addAction(mShowWindowAction);
    mTrayMenu->addAction(mQuitAction);

    mTrayIcon = new QSystemTrayIcon(QIcon(":/systrayicon.png"), this);
    mTrayIcon->setContextMenu(mTrayMenu);
    mTrayIcon->show();

    connect(mShowWindowAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    connect(mQuitAction, SIGNAL(triggered()), this, SLOT(onSysTrayQuit()));
    connect(mTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconClicked(QSystemTrayIcon::ActivationReason)));
}

void LaMainWindow::createConnections() {
    connect(mCloseButton, SIGNAL(clicked()),
            this, SLOT(hide()));

    connect(mLaRunTime, SIGNAL(onShowStatuBarLoadMessage(bool,QString)),
            this, SLOT(showStatusBarLoadMessage(bool,QString)));

    connect(mLaRunTime, SIGNAL(onShowSysTrayMsg(QString,QString)),
            this, SLOT(showSysTrayMessage(QString,QString)));

    connect(mLaRunTime, SIGNAL(SS5ConnectStateChanged(bool)),
            this, SLOT(onTunnelConnectionChanged(bool)));

    connect(mLaRunTime, SIGNAL(dataTransfer(double,double)),
            this, SLOT(onDataTransfer(double,double)));
}

void LaMainWindow::createStatusBarWidgets() {
    mLoadIcon = new QMovie(":/loading.gif");
    mLoadIcon->setScaledSize(QSize(16,16));
    mLoadIconLabel = new QLabel(this);
    mMessageLabel = new QLabel(this);

    QPalette p = mMessageLabel->palette();
    p.setColor(QPalette::WindowText, QColor(Qt::white));
    mMessageLabel->setPalette(p);

    mLoadIconLabel->setMovie(mLoadIcon);

    mDataTransferLabel = new QLabel(this);
    mDataTransferLabel->setPalette(p);

    statusBar()->addPermanentWidget(mLoadIconLabel);
    statusBar()->addPermanentWidget(mMessageLabel);
    statusBar()->addPermanentWidget(mDataTransferLabel);
    statusBar()->setStyleSheet("QStatusBar::item { border: none; }");
    statusBar()->setContentsMargins(10,0,0,5);
}

void LaMainWindow::createLayout() {
    setCentralWidget( new QWidget(this) );

    QVBoxLayout *mainLayout = new QVBoxLayout( centralWidget() );
    mainLayout->setContentsMargins(6,4,6,6);
    centralWidget()->setLayout( mainLayout );

    QLabel *titleLabel = new QLabel(QString("FasterTunnel"), centralWidget());
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont f = titleLabel->font();
    f.setBold(true);
    f.setPixelSize(13);
    titleLabel->setFont(f);
    QPalette p = titleLabel->palette();
    p.setColor(QPalette::WindowText, QColor(Qt::white));
    titleLabel->setPalette(p);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0,0,0,0);
    topLayout->addWidget(titleLabel, 1, Qt::AlignCenter);
    topLayout->addWidget(mCloseButton,0, Qt::AlignRight|Qt::AlignTop);

    mainLayout->addLayout(topLayout, 0);
    mainLayout->addWidget(mLaLoginFrame, 0, Qt::AlignLeft);
    mainLayout->addWidget(mLaMainFrame, 1);
    mainLayout->addWidget(mLaLogFrame, 0);
}

void LaMainWindow::mousePressEvent(QMouseEvent *e) {
    if( e->y() < 30 ) {
        mMouseX = e->x()+10;
        mMouseY = e->y()+30;
    } else {
        mMouseX = -1;
        mMouseY = -1;
    }
}

void LaMainWindow::mouseMoveEvent(QMouseEvent *e) {
    if( mMouseX > 0 && mMouseY > 0 ) {
        move( e->globalX()-mMouseX, e->globalY()-mMouseY );
    }
}

void LaMainWindow::resizeEvent(QResizeEvent *event) {
    QImage image(this->size(), QImage::Format_Mono);
    image.fill(0);

    if(!this->isFullScreen() && !this->isMaximized())
    {
        image.setPixel(0, 0, 1);
        image.setPixel(1, 0, 1);
        image.setPixel(2, 0, 1);
        image.setPixel(3, 0, 1);
        image.setPixel(0, 1, 1);
        image.setPixel(1, 1, 1);
        image.setPixel(0, 2, 1);
        image.setPixel(0, 3, 1);

        image.setPixel(width() - 4, 0, 1);
        image.setPixel(width() - 3, 0, 1);
        image.setPixel(width() - 2, 0, 1);
        image.setPixel(width() - 1, 0, 1);
        image.setPixel(width() - 2, 1, 1);
        image.setPixel(width() - 1, 1, 1);
        image.setPixel(width() - 1, 2, 1);
        image.setPixel(width() - 1, 3, 1);

        image.setPixel(0, height() - 4, 1);
        image.setPixel(0, height() - 3, 1);
        image.setPixel(0, height() - 2, 1);
        image.setPixel(1, height() - 2, 1);
        image.setPixel(0, height() - 1, 1);
        image.setPixel(1, height() - 1, 1);
        image.setPixel(2, height() - 1, 1);
        image.setPixel(3, height() - 1, 1);

        image.setPixel(width() - 2, height() - 2, 1);
        image.setPixel(width() - 1, height() - 2, 1);
        image.setPixel(width() - 4, height() - 1, 1);
        image.setPixel(width() - 3, height() - 1, 1);
        image.setPixel(width() - 2, height() - 1, 1);
        image.setPixel(width() - 1, height() - 1, 1);
    }
    setMask(QBitmap::fromImage(image));
    QMainWindow::resizeEvent(event);
}
