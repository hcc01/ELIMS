QT -= gui
QT +=  network
LIBS += -lpthread libwsock32 libws2_32
TEMPLATE = lib
DEFINES += NETKITS_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CELLBuffer.cpp \
    CELLClient.cpp \
    CELLLog.cpp \
    CELLMsgStream.cpp \
    CELLServer.cpp \
    CELLStream.cpp \
    CELLTask.cpp \
    EasyTcpClient.cpp \
    EasyTcpServer.cpp \
    cbaseuser.cpp \
    server.cpp \
    user.cpp

HEADERS += \
    CELL.h \
    CELLBuffer.h \
    CELLClient.h \
    CELLLog.h \
    CELLMsgStream.h \
    CELLNetWork.hpp \
    CELLSemaphore.hpp \
    CELLServer.h \
    CELLStream.h \
    CELLTask.h \
    CELLThread.h \
    CELLTimestamp.h \
    EasyTcpClient.h \
    EasyTcpServer.h \
    INetEvent.h \
    MessageHeader.h \
    NetKits_global.h \
    cbaseuser.h \
    user.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
CONFIG(debug,debug|release){
    DLLDESTDIR = "../../debug"
     TARGET=Netkitsd
}else{
    DLLDESTDIR = "../../bin"
    TARGET=Netkits
}
DESTDIR="../../lib"

SUBDIRS += \
    NetKits.pro

DISTFILES += \
    NetKits.pro.user
