#ifndef SYSTEMKEYBOARDREADWRITE_H
#define SYSTEMKEYBOARDREADWRITE_H

#pragma once
#include <QObject>
#include <Windows.h>

class SystemKeyboardReadWrite : public QObject
{

    Q_OBJECT
public:
    // Returns singleton instance
    static SystemKeyboardReadWrite * instance();

    // Returns whether the keyboard hook is connected
    bool connected();
    // Connects / Disconnects the keyboard hook
    bool setConnected(bool state);

signals:
    void closeConnectionSignal();

    // Broadcasts a key has been pressed
    void keyPressed(DWORD key);
    // Broadcasts a key has been released
    void keyReleased(DWORD key);

private:
    // Keyboard hook
    HHOOK keyboardHook;

    // Class constructor
    SystemKeyboardReadWrite();

    // Identifies hook activity
    static LRESULT CALLBACK keyboardProcedure(int nCode, WPARAM wParam, LPARAM lParam);

};

#endif // SYSTEMKEYBOARDREADWRITE_H
