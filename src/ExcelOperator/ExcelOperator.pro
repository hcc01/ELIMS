QT += gui axcontainer

TEMPLATE = lib
DEFINES += EXCELOPERATOR_LIBRARY

CONFIG += c++17

include(../QXlsx/QXlsx.pri)
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../Client/qexcel.cpp \
    exceloperator.cpp \
    qwrod.cpp

HEADERS += \
    ../Client/QExcel.h \
    ExcelOperator_global.h \
    exceloperator.h \
    qwrod.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
DESTDIR = "../../lib"
CONFIG(debug,debug|release){
    DLLDESTDIR = "../../debug"
    TARGET = ExcelOperatord
}else{
    DLLDESTDIR = "../../bin"
    TARGET = ExcelOperator
}

