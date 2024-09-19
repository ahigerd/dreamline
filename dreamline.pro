TEMPLATE = app
QT = core widgets
QMAKE_CXXFLAGS += -std=c++17
CONFIG += debug

INCLUDEPATH += src

# store build temporaries in a separate folder
OBJECTS_DIR = .build
RCC_DIR = .build
MOC_DIR = .build

HEADERS += src/gripitem.h   src/polygonitem.h
SOURCES += src/gripitem.cpp src/polygonitem.cpp

SOURCES += src/main.cpp

win32 {
  run.commands = dreamline.exe
}
macx {
  run.commands = open ./dreamline.app
}
else {
  run.commands = ./dreamline
}
run.depends = dreamline
QMAKE_EXTRA_TARGETS += run
