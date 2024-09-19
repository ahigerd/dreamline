#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "glviewport.h"
#include "polygonitem.h"

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define DREAMLINE_VERSION_STRING STRINGIFY(DREAMLINE_VERSION)

int main(int argc, char** argv)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
  // at least on some Wayland systems, a bug causes the application to crash:
  // "QSocketNotifier: Can only be used with threads started with QThread"
  qputenv("QT_QPA_PLATFORM", "xcb");
#endif

  QApplication::setApplicationName("Dreamline");
  QApplication::setApplicationVersion(DREAMLINE_VERSION_STRING);
  QApplication::setOrganizationName("Alkahest");
  QApplication::setOrganizationDomain("com.alkahest");
  QApplication::setDesktopFileName("dreamline.desktop");

  QApplication app(argc, argv);

  QGraphicsView v;
  v.setViewport(new GLViewport(&v));
  QGraphicsScene scene;
  v.setScene(&scene);
  PolygonItem* p = new PolygonItem;
  scene.addItem(p);
  v.resize(800, 600);
  v.show();

  return app.exec();
}
