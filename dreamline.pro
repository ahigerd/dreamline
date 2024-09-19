TEMPLATE = app
QT = core widgets
QMAKE_CXXFLAGS += -std=c++17
CONFIG += debug

INCLUDEPATH += src

# store build temporaries in a separate folder
OBJECTS_DIR = .build
RCC_DIR = .build
MOC_DIR = .build

HEADERS += src/mainwindow.h   src/gripitem.h   src/polygonitem.h
SOURCES += src/mainwindow.cpp src/gripitem.cpp src/polygonitem.cpp

SOURCES += src/main.cpp
