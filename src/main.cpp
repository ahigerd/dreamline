#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "glviewport.h"
#include "polygonitem.h"

int main(int argc, char** argv)
{
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
