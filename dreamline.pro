TEMPLATE = app
QT = core widgets
CONFIG += c++17
CONFIG += debug

INCLUDEPATH += src

# store build temporaries in a separate folder
OBJECTS_DIR = .build
RCC_DIR = .build
MOC_DIR = .build

HEADERS += src/mainwindow.h   src/gripitem.h   src/polygonitem.h
SOURCES += src/mainwindow.cpp src/gripitem.cpp src/polygonitem.cpp

HEADERS += src/glviewport.h   src/edgeitem.h
SOURCES += src/glviewport.cpp src/edgeitem.cpp

HEADERS += src/glbuffer.h
SOURCES += src/main.cpp

RESOURCES += res/shaders.qrc

VERSION = 0.0.1
# If git commands can be run without errors, grab the commit hash
system(git log -1 --pretty=format:) {
  BUILD_HASH = -$$system(git log -1 --pretty=format:%h)
}
else {
  BUILD_HASH =
}

DEFINES += DREAMLINE_VERSION=$${VERSION}$${BUILD_HASH}

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
