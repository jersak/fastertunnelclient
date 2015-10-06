
QT += widgets webkitwidgets network core

RC_FILE = icon.rc

SOURCES += \
    main.cpp \
    Gui/LaLoginFrame.cpp \
    Gui/LaMainFrame.cpp \
    Gui/LaMainWindow.cpp \
    Gui/LaServerListFrame.cpp \
    Gui/LaLogFrame.cpp \
    Model/ModelItem/LaLoginItem.cpp \
    Model/ModelItem/LaServerItem.cpp \
    Model/LaServerTableModel.cpp \
    Model/LaLogTableModel.cpp \
    Util/LaCypher.cpp \
    Engine/LaRunTime.cpp \
    Engine/LaDataIO.cpp \
    Engine/LaNetwork.cpp \
    Thread/LaServerListThread.cpp \
    Thread/LaServerLatencyThread.cpp \
    SingleApplication/SingleApplication.cpp \
    Model/ModelItem/LaTunnelItem.cpp

HEADERS += \
    Gui/LaMainWindow.h \
    Gui/LaMainFrame.h \
    Gui/LaLoginFrame.h \
    Gui/LaServerListFrame.h \
    Gui/LaLogFrame.h \
    Model/ModelItem/LaLoginItem.h \
    Model/ModelItem/LaServerItem.h \
    Model/LaServerTableModel.h \
    Model/LaLogTableModel.h \
    Util/LaCypher.h \
    LaStyleSheet.h \
    Engine/LaRunTime.h \
    Engine/LaDataIO.h \
    Engine/LaNetwork.h \
    Thread/LaServerListThread.h \
    Thread/LaServerLatencyThread.h \
    SingleApplication/SingleApplication.h \
    Model/ModelItem/LaTunnelItem.h

RESOURCES += \
    Resources/resources.qrc
