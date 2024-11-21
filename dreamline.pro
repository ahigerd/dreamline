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

HEADERS += src/dlapplication.h   src/mainwindow.h   src/editorview.h
SOURCES += src/dlapplication.cpp src/mainwindow.cpp src/editorview.cpp

HEADERS += src/glfunctions.h   src/glviewport.h   src/glbuffer.h
SOURCES += src/glfunctions.cpp src/glviewport.cpp src/glbuffer.cpp

HEADERS += src/boundprogram.h   src/mathutil.h   src/dreamproject.h
SOURCES += src/boundprogram.cpp src/mathutil.cpp src/dreamproject.cpp

SOURCES += src/main.cpp

RESOURCES += res/shaders.qrc

for (inc, $$list($$files(*.pri,true))) {
  include($$inc)
}

for (GRP, SRC_GROUPS) {
  for (BASE, $${GRP}_BASE) {
    INCLUDEPATH += $${BASE}
    for (f, $${GRP}_HEADERS) {
      HEADERS += $${BASE}/$$f
    }
    for (f, $${GRP}_SOURCES) {
      SOURCES += $${BASE}/$$f
    }
  }
}

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
  run.commands = dreamline.exe debugmesh.dream
}
macx {
  run.commands = open -a ./dreamline.app ./debugmesh.dream
}
else {
  run.commands = ./dreamline ./debugmesh.dream
}
run.depends = dreamline
QMAKE_EXTRA_TARGETS += run
