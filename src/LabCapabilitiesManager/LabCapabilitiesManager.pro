QT += widgets

TEMPLATE = lib
DEFINES += LABCAPABILITIESMANAGER_LIBRARY

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../Client/ctablemodel.cpp \
    ../Client/mycombobox.cpp \
    ../Client/qjsoncmd.cpp \
    ../Client/tabwigetbase.cpp \
    labcapabilitiesmanagerui.cpp \
    testtypeeditor.cpp

HEADERS += \
    ../Client/ctablemodel.h \
    ../Client/mycombobox.h \
    ../Client/qjsoncmd.h \
    ../Client/tabwigetbase.h \
    LabCapabilitiesManager_global.h \
    labcapabilitiesmanagerui.h \
    testtypeeditor.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

DESTDIR = "../../lib"
CONFIG(debug,debug|release){
    DLLDESTDIR = "../../debug"
    TARGET = LabCapabilitiesManagerd
}else{
    DLLDESTDIR = "../../bin"
    TARGET = LabCapabilitiesManager
}

FORMS += \
    labcapabilitiesmanagerui.ui \
    testtypeeditor.ui
