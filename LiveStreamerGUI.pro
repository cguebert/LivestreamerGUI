QT       += widgets network concurrent

TARGET = LiveStreamerGUI
CONFIG += c++11

TEMPLATE = app


SOURCES += main.cpp \
    MainDialog.cpp \
    StreamsManager.cpp \
    qxt/QxtCheckComboBox.cpp

HEADERS += \
    MainDialog.h \
    StreamsManager.h \
    qxt/QxtCheckComboBox.h

RESOURCES = LiveStreamerGUI.qrc
win32:RC_FILE = LiveStreamerGUI.rc
