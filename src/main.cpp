#include <QApplication>
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

  QApplication app(argc, argv);

  MainWindow v;
  v.resize(800, 600);
  v.show();

  return app.exec();
}
