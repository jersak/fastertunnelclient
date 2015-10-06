#ifndef LOGINFRAME_H
#define LOGINFRAME_H

#include <QFrame>

class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QNetworkReply;
class LaRunTime;
class LaNetwork;

class LaLoginFrame : public QFrame
{
    Q_OBJECT
public:
    explicit LaLoginFrame(LaRunTime *runTime, QWidget *parent = 0);

private slots:
    void onLoginButtonClicked();
    void onLogoutButtonClicked();
    void onLoginResponse(bool validLogin);
    void updateLoginUi(bool logged);
    void onNoServerRespose();
    void onAccountExpirationResponse(QString expirationDate);

private:
    void createWidgets();
    void createLayouts();
    void createConnections();

    void loadData();
    void saveData();

    QLabel *mLoginLabel;
    QLabel *mPasswordLabel;
    QLabel *mExpirationDateLabel;
    QLineEdit *mLoginLineEdit;
    QLineEdit *mPasswordLineEdit;
    QPushButton *mLoginButton;

    QLabel *mLoggedUserLabel;
    QPushButton *mLogoutButton;

    QCheckBox *mSaveLoginCheckBox;
    QCheckBox *mAutoLoginCheckBox;
    QCheckBox *mTrialAccountCheckBox;

    LaRunTime *mLaRunTime;
    LaNetwork *mLaNetwork;
};

#endif // LOGINFRAME_H
