#-------------------------------------------------
#
# Project created by QtCreator 2022-11-09T10:30:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtXmlRPC
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14
QMAKE_CXXFLAGS_RELEASE = -Od -ZI -MD
QMAKE_LFLAGS_RELEASE = /DEBUG /INCREMENTAL:NO

SOURCES += \
        dbservicethread.cpp \
        dbthread.cpp \
        main.cpp \
        mainwindow.cpp \
        rpcservicethread.cpp \
        title/TitleBar.cpp \
        title/titlewidget.cpp

HEADERS += \
        GlobDef.h \
        dbservicethread.h \
        dbthread.h \
        mainwindow.h \
        procact.h \
        procact_DB.h \
        rpcservicethread.h \
        title/TitleBar.h \
        title/titlewidget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += .\include\curl-7.80.0
INCLUDEPATH += .\include
INCLUDEPATH += .\include\jsoncpp\json

DEFINES += QSLOG_IS_SHARED_LIBRARY_IMPORT
DEFINES += QS_LOG_LINE_NUMBERS
DEFINES += HTTP_ONLY
DEFINES += CURL_STATICLIB
LIBS += -lws2_32 -lwldap32 -lCrypt32 -ladvapi32 -lWininet

LIBS += -L$$PWD\lib\x64\ -lQsLog2
LIBS += -L$$PWD\lib\x64\ -lDMDBAccess
LIBS += -L$$PWD\lib\curl-7.80.0\x64\ -llibcurl
LIBS += -L$$PWD\lib\xml-rpc\x64 -llibxmlrpc -llibxmlrpc_abyss -llibxmlrpc_abyss++ \
                                -llibxmlrpc_client -llibxmlrpc_server -llibxmlrpc_server_abyss \
                                -llibxmlrpc_server_cgi -llibxmlrpc_server_w32httpsys \
                                -llibxmlrpc_util -llibxmlrpc_util++ -llibxmlrpc_xmlparse \
                                -llibxmlrpc_xmltok -llibxmlrpc++
LIBS += -L$$PWD\lib\jsoncpp -ljsoncpp_static

RESOURCES += \
    rc.qrc
