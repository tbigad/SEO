#-------------------------------------------------
#
# Project created by QtCreator 2014-09-24T22:52:15
#
#-------------------------------------------------
include(..\SEO\src\QtXlsxWriter-master\src\xlsx\qtxlsx.pri)
#Qt Xlsx Module from http://qtxlsx.debao.me/

QT += network core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Seo
TEMPLATE = app
QMAKE_CXXFLAGS += -std=gnu++11
SOURCES +=  ../SEO/TS_project/*.cpp \
            ../SEO/WF_project/*.cpp \
            ../SEO/*.cpp \

LIBS += -L/usr/local/lib/ -lxml2
HEADERS  += ../SEO/TS_project/*.h \
            ../SEO/WF_project/*.h \
            ../SEO/*.h \

HEADERS += D:\lib\libxml2-2.7.7\include\*.h
SOURCES += D:\lib\libxml2-2.7.7\xmlmemory.c

FORMS    += ../SEO/TS_project/*.ui \
            ../SEO/WF_project/*.ui \
            ../SEO/*.ui

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
