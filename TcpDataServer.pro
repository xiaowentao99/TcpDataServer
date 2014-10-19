#-------------------------------------------------
#
# Project created by QtCreator 2014-08-28T21:29:42
#
#-------------------------------------------------

QT       += core
QT       += network

QT       -= gui

TARGET = TcpDataServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    tcpserver.cpp \
    connectionnode.cpp \
    common.cpp

HEADERS += \
    tcpserver.h \
    common.h \
    connectionnode.h
