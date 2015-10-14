#include <Gui/lamainwindow.h>
#include <Engine/LaRunTime.h>
#include <SingleApplication/singleapplication.h>
#include <QMessageBox>
#include <QFile>
#include <QtCore>
#include <QSharedMemory>
#include <Util/SystemKeyboardReadWrite.h>

#include "windows.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSharedMemory shared("61BB200D-3579-453e-9044-");
    if(shared.create(512,QSharedMemory::ReadWrite)==true) {
    } else exit(0);

    QFile f;
    f.setFileName(qApp->applicationDirPath() + "/FtcMonitor.exe");
    if(!f.exists()) {
        QMessageBox msg;
        msg.setWindowTitle("Falha de inicialização");
        msg.setText("Arquivos faltando: FtcMonitor.exe");
        msg.exec();
        return 0;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    app.addLibraryPath(QCoreApplication::applicationDirPath());

    LaRunTime mRunTime;

    SystemKeyboardReadWrite::instance()->setConnected(true);

    QObject::connect(SystemKeyboardReadWrite::instance(), SIGNAL(closeConnectionSignal()),
                     &mRunTime, SLOT(onDisconnectShorcutPressed()));

    LaMainWindow w(&mRunTime);
    w.show();

    return app.exec();
}
