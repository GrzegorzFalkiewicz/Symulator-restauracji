QT += widgets

CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = RestaurantSimulator

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/simulationengine.cpp \
    src/graphicsobjects.cpp \
    src/restaurantview.cpp

HEADERS += \
    src/mainwindow.h \
    src/order.h \
    src/simulationengine.h \
    src/graphicsobjects.h \
    src/restaurantview.h
