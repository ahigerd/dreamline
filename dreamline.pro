TEMPLATE = app
QT = core widgets widgets-private
CONFIG += c++17
CONFIG += release optimze_full
CONFIG -= optimize_size
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS += -ffast-math -msse2 -O3 -ggdb

INCLUDEPATH += src

# store build temporaries in a separate folder
OBJECTS_DIR = .build
RCC_DIR = .build
MOC_DIR = .build

HEADERS += src/mainwindow.h   src/gripitem.h   src/meshitem.h   src/tool.h
SOURCES += src/mainwindow.cpp src/gripitem.cpp src/meshitem.cpp src/tool.cpp

HEADERS += src/editorview.h   src/glviewport.h   src/edgeitem.h   src/markeritem.h
SOURCES += src/editorview.cpp src/glviewport.cpp src/edgeitem.cpp src/markeritem.cpp

HEADERS += src/glbuffer.h   src/boundprogram.h   src/dreamproject.h
SOURCES += src/glbuffer.cpp src/boundprogram.cpp src/dreamproject.cpp


HEADERS += src/tools/movevertex.h   src/tools/moveedge.h   src/tools/color.h   src/tools/split.h
SOURCES += src/tools/movevertex.cpp src/tools/moveedge.cpp src/tools/color.cpp src/tools/split.cpp

HEADERS += src/mathutil.h   src/dlapplication.h
SOURCES += src/mathutil.cpp src/dlapplication.cpp

SOURCES += src/main.cpp src/meshitem_polygon.cpp

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
