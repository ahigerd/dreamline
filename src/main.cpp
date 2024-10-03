#include <QApplication>
#include "mainwindow.h"
#include "commandline.h"

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

  CommandLine opt;
  opt.hideOption("qmljsdebugger");
  /* TODO:
  opt.addPositionalArgument(
    "paths",
    QCoreApplication::translate("main", "A list of files to open (optional)."),
    QCoreApplication::translate("main", "[paths...]")
  );
  */
  opt.addPositionalArgument(
    "path",
    QCoreApplication::translate("main", "A file to open (optional).")
  );

  int status = opt.process(argc, argv);
  if (status < 0) {
    return 0;
  } else if (status) {
    return status;
  }

  QPair<int, char**> modifiedArgv = opt.modifiedArgv();
  QApplication app(modifiedArgv.first, modifiedArgv.second);

  MainWindow v;
  v.resize(800, 600);
  v.show();

  if (opt.positionalArguments().length()) {
    v.openFile(opt.positionalArguments().first());
  }

  return app.exec();
}
