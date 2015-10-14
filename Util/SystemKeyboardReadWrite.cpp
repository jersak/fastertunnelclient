#include "systemkeyboardreadwrite.h"

#include <QDebug>
#include "windows.h"
#include "shellapi.h"

SystemKeyboardReadWrite::SystemKeyboardReadWrite() :
    QObject()
{
    // Assign to null
    keyboardHook = NULL;
}


LRESULT CALLBACK SystemKeyboardReadWrite::keyboardProcedure(int nCode, WPARAM wParam, LPARAM lParam)
{
    // Check for a key down press
    if (nCode == HC_ACTION)
    {
        if (wParam == WM_KEYDOWN)
        {
            KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
            if (((DWORD) pKeyboard->vkCode) == 81 && (GetAsyncKeyState(VK_LSHIFT) < 0 && GetAsyncKeyState(VK_CONTROL) < 0)) {
                qDebug() << "Key Param: CTRL + SHIFT + Q";
                emit SystemKeyboardReadWrite::instance()->closeConnectionSignal();
            }
        }
        //reconnect
        instance()->setConnected( false );
        instance()->setConnected( true );
    }

    return false;
}

bool SystemKeyboardReadWrite::connected() {
    return keyboardHook;
}

bool SystemKeyboardReadWrite::setConnected(bool state)
{
    if(state && keyboardHook == NULL)
    {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProcedure, GetModuleHandle(NULL), 0);

        return keyboardHook;
    }
    else
    {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;

        return keyboardHook;
    }
}

SystemKeyboardReadWrite* SystemKeyboardReadWrite::instance()
{
    static SystemKeyboardReadWrite* pKeyboardReadWriteInstance = new SystemKeyboardReadWrite();
    return pKeyboardReadWriteInstance;
}
