#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "polygonitem.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  QGraphicsView v;
  QGraphicsScene scene;
  v.setScene(&scene);
  PolygonItem* p = new PolygonItem;
  scene.addItem(p);
  v.resize(800, 600);
  v.show();

  return app.exec();
}
