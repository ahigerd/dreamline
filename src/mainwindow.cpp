#include "mainwindow.h"
#include "polygonitem.h"
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), graphicsView(new QGraphicsView(this))
{
  // Create a scene
  QGraphicsScene *scene = new QGraphicsScene(this);

  PolygonItem* p = new PolygonItem;
  scene->addItem(p);
  // Set the scene on the QGraphicsView
  graphicsView->setScene(scene);

  // Set the QGraphicsView as the central widget
  setCentralWidget(graphicsView);
}

MainWindow::~MainWindow() {
  // No need to manually delete graphicsView, Qt's parent-child system handles it.
}

