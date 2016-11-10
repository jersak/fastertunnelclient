#include "LaLoginFrame.h"

#include <Model/ModelItem/laloginitem.h>
#include <Engine/LaDataIO.h>
#include <Engine/LaRunTime.h>
#include <Engine/LaNetwork.h>
#include <LaStyleSheet.h>

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QDateTime>
#include <QMovie>
#include <QGroupBox>
#include <QPropertyAnimation>

LaLoginFrame::LaLoginFrame(LaRunTime *runTime, QWidget *parent) :
    QFrame(parent)
{
    mLaRunTime = runTime;

    createWidgets();
    createLayouts();
    createConnections();
    loadData();

    setObjectName("GlossyFrame");
    setStyleSheet(LaLoginStyleSheet);

    // Faz auto login caso configurado
    if( mAutoLoginCheckBox->isChecked() )
        onLoginButtonClicked();
}

void LaLoginFrame::onLoginButtonClicked() {
    // Logar com contra trial
//    if( mTrialAccountCheckBox->isChecked() ) {
//        mLaRunTime->setUsingTrialAccount(true);
//        onLoginResponse(true);
//        return;
//    }

    mLaRunTime->setUsingTrialAccount(false);

    mLaRunTime->showStatusBarLoadMessage(true, "Autenticando.");
    mLaNetwork = new LaNetwork(this);

    connect(mLaNetwork, SIGNAL(loginResponse(bool)),
            this, SLOT(onLoginResponse(bool)));

    connect(mLaNetwork, SIGNAL(noResponseFromLoginServer()),
            this, SLOT(onNoServerRespose()));

    connect(mLaNetwork, SIGNAL(accountExpirationResponse(QString)),
            this, SLOT(onAccountExpirationResponse(QString)));

    QString username = mLoginLineEdit->text();
    QString password = mPasswordLineEdit->text();
    QString hash = QString::number(QDateTime::currentDateTime().toTime_t());

    // Armazena o has local para futuras comparações
    mLaRunTime->setCurrentHash(hash);

    QString hwid = NULL;

    if (mTrialAccountCheckBox->isChecked()){
        hwid = mLaRunTime->checkHwId();
    }

    mLaNetwork->tryLogin(username,password,hash,hwid);
}

void LaLoginFrame::onLogoutButtonClicked() {
    mLaRunTime->changeLoginState(false);
}

void LaLoginFrame::onLoginResponse(bool validLogin) {
    mLaRunTime->showStatusBarLoadMessage(false);

    if( !validLogin ) {
        mLaRunTime->showLogMessage("Falha de autenticação");

        mLaRunTime->setCurrentHash(QString()); // Zera o hash local
        QMessageBox msg;
        msg.setWindowTitle(QString("Falha de autenticação."));
        msg.setText("Usuário ou senha incorretos, ou a sua conta ou teste expirou.");
        msg.exec();
        return;
    }

    // Guarda o ultimo horário de login para gerar arquivo de log
    mLaRunTime->startNewLogFile();

    if( !mLaRunTime->isUsingTrialAccount() ) {
        mLaRunTime->setCurrentUser(mLoginLineEdit->text());
        mLaRunTime->showLogMessage("Login efetuado com sucesso.");
        mLaNetwork->requestAccountExpirationDate(mLoginLineEdit->text());
    } else mLaRunTime->showLogMessage("Login efetuado com sucesso [TrialAccount].");

    mLaRunTime->changeLoginState(validLogin);
    if(!mLaRunTime->isUsingTrialAccount())
        saveData();
}

void LaLoginFrame::onNoServerRespose() {
    mLaRunTime->showStatusBarLoadMessage(false);
    mLaRunTime->showLogMessage("Falha de autenticação. Não foi possível conectar ao servidor.");

    mLaRunTime->setCurrentHash(QString()); // Zera o hash local
    QMessageBox msg;
    msg.setWindowTitle(QString("Falha de autenticação."));
    msg.setText("Não foi possível conectar ao servidor.");
    msg.exec();
}

void LaLoginFrame::onAccountExpirationResponse(QString expirationDate) {
    mExpirationDateLabel->setText(expirationDate);
}

void LaLoginFrame::createWidgets() {
    // When logged in
    mLoggedUserLabel = new QLabel(this);
    mLoggedUserLabel->setVisible(false);
    QFont f = mLoggedUserLabel->font();
    f.setPointSize(15);
    mLoggedUserLabel->setStyleSheet("color: lightgreen;");
    mLoggedUserLabel->setFont(f);

    // When logged out
    mLoginLabel = new QLabel("Usuário", this);
    mPasswordLabel = new QLabel("Senha", this);
    mExpirationDateLabel = new QLabel(this);
    mExpirationDateLabel->setVisible(false);
    mLoginLineEdit = new QLineEdit(this);
    mLoginLineEdit->setFixedWidth(150);
    mPasswordLineEdit = new QLineEdit(this);
    mPasswordLineEdit->setEchoMode(QLineEdit::Password);
    mPasswordLineEdit->setFixedWidth(150);
    mSaveLoginCheckBox = new QCheckBox("Salvar Usuário/Senha", this);
    mAutoLoginCheckBox = new QCheckBox("Auto Login", this);
    mTrialAccountCheckBox = new QCheckBox("Conta Teste", this);
    mTrialAccountCheckBox->setDisabled(false);
    mLoginButton = new QPushButton("Login", this);
    mLoginButton->setFixedWidth(80);
    mLogoutButton = new QPushButton("Logout");
    mLogoutButton->setVisible(false);
    mLogoutButton->setFixedWidth(80);
}

void LaLoginFrame::createLayouts() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    setLayout(mainLayout);

    QGroupBox *loginBox = new QGroupBox("Login", this);
    QGridLayout *loginLayout = new QGridLayout(loginBox);
    loginLayout->setContentsMargins(10,5,10,5);

    loginLayout->addWidget(mLoginLabel, 0, 0, Qt::AlignLeft);
    loginLayout->addWidget(mPasswordLabel, 0, 1, Qt::AlignLeft);
    loginLayout->addWidget(mExpirationDateLabel, 0, 1, 1, 2, Qt::AlignLeft);

    loginLayout->addWidget(mLoginLineEdit, 1, 0, Qt::AlignLeft);
    loginLayout->addWidget(mPasswordLineEdit, 1, 1, Qt::AlignLeft);
    loginLayout->addWidget(mLoginButton, 1, 2);

    loginLayout->addWidget(mLogoutButton, 1, 2);
    loginLayout->addWidget(mLoggedUserLabel, 1, 0, Qt::AlignLeft);

    loginLayout->addWidget(mSaveLoginCheckBox, 2, 0, Qt::AlignLeft);
    loginLayout->addWidget(mAutoLoginCheckBox, 2, 1, Qt::AlignLeft);
    loginLayout->addWidget(mTrialAccountCheckBox, 2, 2, Qt::AlignLeft);

    loginBox->setLayout(loginLayout);
    loginBox->setStyleSheet("color: white; ");
    mPasswordLineEdit->setStyleSheet("color: black;");
    mLoginLineEdit->setStyleSheet("color: black;");

    mainLayout->addWidget( loginBox, 0, Qt::AlignRight );
    loginBox->setFixedSize(406, 95);
}

void LaLoginFrame::createConnections() {
    connect(mSaveLoginCheckBox, SIGNAL(toggled(bool)),
            mAutoLoginCheckBox, SLOT(setEnabled(bool)));

    connect(mLoginButton, SIGNAL(clicked()),
            this, SLOT(onLoginButtonClicked()));

    connect(mLogoutButton, SIGNAL(clicked()),
            this, SLOT(onLogoutButtonClicked()));

    connect(mLaRunTime, SIGNAL(onLoginStateChange(bool)),
            this, SLOT(updateLoginUi(bool)));
}

void LaLoginFrame::loadData() {
    LaLoginItem mLaLoginItem = LaDataIO::readLoginInfo();

    mLoginLineEdit->setText( mLaLoginItem.userName() );
    mPasswordLineEdit->setText( mLaLoginItem.password() );
    mSaveLoginCheckBox->setChecked( mLaLoginItem.saveLogin() );
    mAutoLoginCheckBox->setChecked( mLaLoginItem.autoLogin() );
}

void LaLoginFrame::saveData() {
    if( mSaveLoginCheckBox->isChecked() ) {
        if( mTrialAccountCheckBox->isChecked() )
            return;

        LaLoginItem *mLaLoginItem = new LaLoginItem();

        mLaLoginItem->setUserName(mLoginLineEdit->text());
        mLaLoginItem->setPassword(mPasswordLineEdit->text());
        mLaLoginItem->setSaveLogin(mSaveLoginCheckBox->isChecked());
        mLaLoginItem->setAutoLogin(mAutoLoginCheckBox->isChecked());

        LaDataIO::writeLoginInfo(mLaLoginItem);
        mLaRunTime->showLogMessage("Dados de login serão lembrados.");
    } else {
        LaLoginItem *mLaLoginItem = new LaLoginItem();
        mLaLoginItem->setSaveLogin(false);

        LaDataIO::writeLoginInfo(mLaLoginItem);
        mLaRunTime->showLogMessage("Dados de login não serão lembrados.");
    }
}

void LaLoginFrame::updateLoginUi(bool logged) {
    mPasswordLabel->setVisible(!logged);
    mLoginLineEdit->setVisible(!logged);
    mPasswordLineEdit->setVisible(!logged);
    mSaveLoginCheckBox->setEnabled(!logged);
    mAutoLoginCheckBox->setEnabled(!logged);
    mTrialAccountCheckBox->setEnabled(!logged);
    mLoginButton->setVisible(!logged);
    mLoggedUserLabel->setVisible(logged);

    if(mLaRunTime->isUsingTrialAccount())
        mLoggedUserLabel->setText("Trial Account");
    else {
        mLoggedUserLabel->setText(mLaRunTime->currentUser());
        mExpirationDateLabel->setVisible(logged);
    }

    mLogoutButton->setVisible(logged);
}
