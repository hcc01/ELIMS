QT -= gui

LIBS += -lpthread libwsock32 libws2_32
TEMPLATE = lib
DEFINES += TCPSERVER_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    TcpServer.cpp

HEADERS += \
    TcpServer_global.h \
    TcpServer.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += $$PWD/../../include

CONFIG(debug,debug|release){
    DESTDIR += ../../debug
}else{
    DESTDIR += ../../bin
}



INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include
