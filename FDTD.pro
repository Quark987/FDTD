#-------------------------------------------------
#
# Project created by QtCreator 2015-08-16T13:30:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = FDTD
TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle

ICON = icon.icns

SOURCES += main.cpp\
        fdtd.cpp \
    qcustomplot.cpp \
    preferences.cpp \
    settings.cpp \
    field.cpp \
    area.cpp \
    pmlboundary.cpp \
    currentsource.cpp \
    sourcesettings.cpp \
    materialdefinition.cpp \
    materialsettings.cpp \
    planewave.cpp \
    point.cpp \
    tfsfsettings.cpp \
    pointinpolygon.cpp \
    sensordefinition.cpp \
    sensorresult.cpp \
    sensorsettings.cpp \
    coordinatetable.cpp \
    inputrange.cpp \
    sgfield.cpp \
    sginterface.cpp \
    sgsettings.cpp

HEADERS  += fdtd.h \
    qcustomplot.h \
    preferences.h \
    settings.h \
    field.h \
    area.h \
    pmlboundary.h \
    currentsource.h \
    sourcesettings.h \
    materialdefinition.h \
    materialsettings.h \
    planewave.h \
    point.h \
    tfsfsettings.h \
    pointinpolygon.h \
    sensordefinition.h \
    sensorresult.h \
    sensorsettings.h \
    coordinatetable.h \
    inputrange.h \
    sgfield.h \
    sginterface.h \
    sgsettings.h

FORMS    += fdtd.ui \
    preferences.ui \
    sourcesettings.ui \
    materialsettings.ui \
    tfsfsettings.ui \
    sensorresult.ui \
    sensorsettings.ui \
    inputrange.ui \
    sgsettings.ui

DISTFILES += \
    Preferences.qml

RESOURCES += \
    resources.qrc

#INCLUDEPATH += "C:/fftwMinGW"

QMAKE_CXXFLAGS += -O3
#QMAKE_CXXFLAGS+= -openmp
#QMAKE_LFLAGS +=  -openmp

LIBS += -L/usr/local/lib -lfftw3
