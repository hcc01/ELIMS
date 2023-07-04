QT       += core gui network sql axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
LIBS += -lpthread libwsock32 libws2_32


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
../../depends/CELLBuffer.cpp \
../../depends/CELLClient.cpp \
../../depends/CELLLog.cpp \
../../depends/CELLMsgStream.cpp \
../../depends/CELLServer.cpp \
../../depends/CELLStream.cpp \
../../depends/CELLTask.cpp \
../../depends/EasyTcpClient.cpp \
../../depends/EasyTcpServer.cpp \
../../depends/server.cpp \
../../depends/user.cpp \
    cclient.cpp \
    ctablemodel.cpp \
    cuser.cpp \
    implementingstandardeditor.cpp \
    itemsselectdlg.cpp \
    loginui.cpp \
    main.cpp \
    mainwindow.cpp \
    modinitui.cpp \
    mycombobox.cpp \
    processmanager.cpp \
    qdoubleedit.cpp \
    qexcel.cpp \
    qjsoncmd.cpp \
    qmeasurementunit.cpp \
    standardsmanager.cpp \
    tabwigetbase.cpp \
    testitemmanager.cpp \
    uniteditbox.cpp \
    unitsc.cpp

HEADERS += \
../../depends/CELL.h \
../../depends/CELLBuffer.h \
../../depends/CELLClient.h \
../../depends/CELLLog.h \
../../depends/CELLMsgStream.h \
../../depends/CELLNetWork.hpp \
../../depends/CELLSemaphore.hpp \
../../depends/CELLServer.h \
../../depends/CELLStream.h \
../../depends/CELLTask.h \
../../depends/CELLThread.h \
../../depends/CELLTimestamp.h \
../../depends/EasyTcpClient.h \
../../depends/EasyTcpServer.h \
../../depends/INetEvent.h \
../../depends/MessageHeader.h \
../../depends/user.h \
    QExcel.h \
    cclient.h \
    ctablemodel.h \
    cuser.h \
    implementingstandardeditor.h \
    itemsselectdlg.h \
    loginui.h \
    mainwindow.h \
    modinitui.h \
    mycombobox.h \
    processmanager.h \
    qdoubleedit.h \
    qjsoncmd.h \
    qmeasurementunit.h \
    standardsmanager.h \
    tabfactory.h \
    tabwigetbase.h \
    testitemmanager.h \
    uniteditbox.h \
    unitsc.h

FORMS += \
    implementingstandardeditor.ui \
    itemsselectdlg.ui \
    loginui.ui \
    mainwindow.ui \
    modinitui.ui \
    standardsmanager.ui \
    testitemmanager.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
#INCLUDEPATH += $$PWD/../../include
#DEPENDPATH += $$PWD/../../depends

CONFIG(debug,debug|release){
    DESTDIR += ../../debug
}else{
    DESTDIR += ../../bin
}

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/ -lNetKits
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/ -lNetKitsd
#else:unix: LIBS += -L$$PWD/../../lib/ -lNetKits

#INCLUDEPATH += $$PWD/../netkits
#DEPENDPATH += $$PWD/../netkits

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/ -lRMManager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/ -lRMManagerd
else:unix: LIBS += -L$$PWD/../../lib/ -lRMManager

INCLUDEPATH += $$PWD/../rmmanager
DEPENDPATH += $$PWD/../rmmanager

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/ -lEmployeeManager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/ -lEmployeeManagerd
else:unix: LIBS += -L$$PWD/../../lib/ -lEmployeeManager

INCLUDEPATH += $$PWD/../employeemanager
DEPENDPATH += $$PWD/../employeemanager

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/ -lDBManagerUI
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/ -lDBManagerUId
else:unix: LIBS += -L$$PWD/../../lib/ -lDBManagerUI

INCLUDEPATH += $$PWD/../dbmanagerui
DEPENDPATH += $$PWD/../dbmanagerui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/ -lTaskSheetManager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/ -lTaskSheetManagerd
else:unix: LIBS += -L$$PWD/../../lib/ -lTaskSheetManager

INCLUDEPATH += $$PWD/../tasksheetmanager
DEPENDPATH += $$PWD/../tasksheetmanager

RESOURCES += \
    res.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/ -lLabCapabilitiesManager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/ -lLabCapabilitiesManagerd
else:unix: LIBS += -L$$PWD/../../lib/ -lLabCapabilitiesManager

INCLUDEPATH += $$PWD/../labcapabilitiesmanager
DEPENDPATH += $$PWD/../labcapabilitiesmanager
