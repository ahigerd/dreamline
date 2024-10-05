#include "dlapplication.h"
#include "mainwindow.h"

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define DREAMLINE_VERSION_STRING STRINGIFY(DREAMLINE_VERSION)

int main(int argc, char** argv)
{
  QApplication::setApplicationName("Dreamline");
  QApplication::setApplicationVersion(DREAMLINE_VERSION_STRING);
  QApplication::setOrganizationName("Alkahest");
  QApplication::setOrganizationDomain("com.alkahest");
  QApplication::setDesktopFileName("dreamline.desktop");

  DLApplication app(argc, argv);
  int exitCode = 0;
  bool shouldExit = app.processCommandLine(&exitCode);
  if (shouldExit) {
    return exitCode;
  }

  MainWindow v;
  v.resize(800, 600);
  v.show();

  if (app.positionalArguments().count()) {
    // TODO: open more than one file
    v.openFile(app.positionalArguments().first());
  }

  return app.exec();
}
