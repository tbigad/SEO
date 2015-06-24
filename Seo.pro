#-------------------------------------------------
#
# Project created by QtCreator 2014-09-24T22:52:15
#
#-------------------------------------------------
include(/home/daek/Qt/xlsx/src/xlsx/qtxlsx.pri)
QT       += core gui sql
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Seo
TEMPLATE = app
QMAKE_CXXFLAGS += -std=gnu++11
SOURCES +=  ../Seo/TS_project/*.cpp \
            ../Seo/WF_project/*.cpp \
            ../Seo/*.cpp \

LIBS += -L/usr/local/lib/ -lxml2
HEADERS  += ../Seo/TS_project/*.h \
            ../Seo/WF_project/*.h \
            ../Seo/*.h \

HEADERS += /home/daek/Документы/sandbox9/third_party/libxml2-2.7.7/include/*.h
SOURCES += /home/daek/Документы/sandbox9/third_party/libxml2-2.7.7/xmlmemory.c

FORMS    += ../Seo/TS_project/*.ui \
            ../Seo/WF_project/*.ui \
            ../Seo/*.ui

RESOURCES += \
    res.qrc

unix:!macx: LIBS += -L$$PWD/../../sandbox9/build/linux-release-gcc-any/static/ -lopc

INCLUDEPATH += $$PWD/../../sandbox9/build/linux-release-gcc-any/static
DEPENDPATH += $$PWD/../../sandbox9/build/linux-release-gcc-any/static

unix:!macx: PRE_TARGETDEPS += $$PWD/../../sandbox9/build/linux-release-gcc-any/static/libopc.a

unix:!macx: LIBS += -L$$PWD/../../sandbox9/build/linux-release-gcc-any/static/ -lmce

INCLUDEPATH += $$PWD/../../sandbox9/build/linux-release-gcc-any/static
DEPENDPATH += $$PWD/../../sandbox9/build/linux-release-gcc-any/static

unix:!macx: PRE_TARGETDEPS += $$PWD/../../sandbox9/build/linux-release-gcc-any/static/libmce.a

unix:!macx: LIBS += -L$$PWD/../../sandbox9/build/linux-release-gcc-any/static/ -lplib

INCLUDEPATH += $$PWD/../../sandbox9/build/linux-release-gcc-any/static
DEPENDPATH += $$PWD/../../sandbox9/build/linux-release-gcc-any/static

unix:!macx: PRE_TARGETDEPS += $$PWD/../../sandbox9/build/linux-release-gcc-any/static/libplib.a

unix:!macx: LIBS += -L$$PWD/../../sandbox9/third_party/zlib-1.2.5/ -lz

INCLUDEPATH += $$PWD/../../sandbox9/third_party/zlib-1.2.5
DEPENDPATH += $$PWD/../../sandbox9/third_party/zlib-1.2.5

unix:!macx: PRE_TARGETDEPS += $$PWD/../../sandbox9/third_party/zlib-1.2.5/libz.a

DISTFILES +=
