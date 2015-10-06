#ifndef LALOGINITEM_H
#define LALOGINITEM_H

#include <QString>

class LaLoginItem
{
public:
    LaLoginItem();

    QString userName() { return mUserName; }
    QString password() { return mPassword; }
    bool saveLogin() { return mSaveLogin; }
    bool autoLogin() { return mAutoLogin; }

    void setUserName( QString userName ) { mUserName = userName; }
    void setPassword( QString password ) { mPassword = password; }
    void setSaveLogin( bool saveLogin ) { mSaveLogin = saveLogin; }
    void setAutoLogin( bool autoLogin ) { mAutoLogin = autoLogin; }

private:
    QString mUserName;
    QString mPassword;

    bool mSaveLogin;
    bool mAutoLogin;
};

#endif // LALOGINITEM_H
