TEMPLATE = app
QT = core widgets
CONFIG += c++17
CONFIG += debug

INCLUDEPATH += src

# store build temporaries in a separate folder
OBJECTS_DIR = .build
RCC_DIR = .build
MOC_DIR = .build

HEADERS += src/glviewport.h   src/gripitem.h   src/polygonitem.h
SOURCES += src/glviewport.cpp src/gripitem.cpp src/polygonitem.cpp

HEADERS += src/glbuffer.h
SOURCES += src/main.cpp

RESOURCES += res/shaders.qrc

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
