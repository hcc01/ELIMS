QT += widgets

TEMPLATE = lib
DEFINES += EMPLOYEEMANAGER_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../Client/qjsoncmd.cpp \
    ../Client/tabwigetbase.cpp \
    employeemanager.cpp \
    employeemanageui.cpp

HEADERS += \
    ../Client/qjsoncmd.h \
    ../Client/tabwigetbase.h \
    EmployeeManager_global.h \
    employeemanager.h \
    employeemanageui.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    employeemanageui.ui
    DESTDIR = "../../lib"
    CONFIG(debug,debug|release){
        DLLDESTDIR = "../../debug"
        TARGET = EmployeeManagerd
    }else{
        DLLDESTDIR = "../../bin"
        TARGET = EmployeeManager
    }
