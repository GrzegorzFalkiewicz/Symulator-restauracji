QT += widgets network

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

# --- Dodatkowe pliki klienta (mozna skompilowac jako osobny projekt) ---
# src/client_main.cpp
# src/clientwindow.h
# src/clientwindow.cpp
