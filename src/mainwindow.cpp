#include "mainwindow.h"
#include "polygonitem.h"
#include "glviewport.h"
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QGestureEvent>
#include <QPinchGesture>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), graphicsView(new QGraphicsView(this))
{
  setMenuBar(new QMenuBar(this));
  makeFileMenu();

  viewport = new GLViewport(graphicsView);
  graphicsView->setViewport(viewport);
  graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  viewport->grabGesture(Qt::PinchGesture);
  viewport->installEventFilter(this);

  setCentralWidget(graphicsView);

  fileNew();
}

MainWindow::~MainWindow() {
}

void MainWindow::makeFileMenu()
{
  QMenu* fileMenu = new QMenu(tr("&File"), this);
  menuBar()->addMenu(fileMenu);
  fileMenu->addAction(tr("&New"), this, SLOT(fileNew()), QStringLiteral("Ctrl+N"));
  fileMenu->addAction(tr("&Open..."), this, SLOT(fileOpen()), QStringLiteral("Ctrl+O"));
  fileMenu->addAction(tr("&Save"), this, SLOT(fileSave()), QStringLiteral("Ctrl+S"));
  fileMenu->addAction(tr("Save &As..."), this, SLOT(fileSaveAs()), QStringLiteral("Ctrl+Shift+S"));
  fileMenu->addSeparator();
  fileMenu->addAction(tr("&Export..."), this, SLOT(fileExport()), QStringLiteral("Ctrl+E"));
  fileMenu->addSeparator();
  fileMenu->addAction(tr("E&xit"), qApp, SLOT(quit()));
}

void MainWindow::fileNew()
{
  savePath.clear();

  QGraphicsScene* oldScene = graphicsView->scene();

  QGraphicsScene* scene = new QGraphicsScene(this);
  graphicsView->setScene(scene);
  PolygonItem* p = new PolygonItem;
  scene->addItem(p);

  delete oldScene;
}

void MainWindow::fileOpen()
{
  QString path = QFileDialog::getOpenFileName(this, "Open DreamLine File", QString(), "DreamLine Files (*.dream)");
  if (path.isEmpty()) {
    return;
  }
  openFile(path);
}

void MainWindow::openFile(const QString& path)
{
  // TODO
  savePath = path;
}

void MainWindow::fileSave()
{
  if (savePath.isEmpty()) {
    fileSaveAs();
    return;
  }
  saveFile(savePath);
}

void MainWindow::fileSaveAs()
{
  QString path = QFileDialog::getSaveFileName(this, "Save DreamLine File", savePath, "DreamLine Files (*.dream)");
  if (path.isEmpty()) {
    return;
  }
  saveFile(path);
}

void MainWindow::saveFile(const QString& path)
{
  savePath = path;
  bool ok = false; // TODO
  if (!ok) {
    QMessageBox::warning(this, tr("Error writing DreamLine file"), tr("%1 could not be saved.").arg(path));
  }
}

void MainWindow::fileExport()
{
  QString path = QFileDialog::getSaveFileName(this, "Export DreamLine File", exportPath, "PNG Files (*.png)");
  if (path.isEmpty()) {
    return;
  }
  exportFile(path);
}

void MainWindow::exportFile(const QString& path)
{
  exportPath = path;
  bool ok = false; // TODO
  if (!ok) {
    QMessageBox::warning(this, tr("Error exporting DreamLine file"), tr("%1 could not be saved.").arg(path));
  }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == viewport) {
    if (event->type() == QEvent::Gesture) {
      QGestureEvent* gesture = static_cast<QGestureEvent*>(event);
      QPinchGesture* pinch = static_cast<QPinchGesture*>(gesture->gesture(Qt::PinchGesture));
      if (pinch) {
        pinchGesture(pinch);
      }
    }
  }
  return false;
}

void MainWindow::pinchGesture(QPinchGesture* gesture)
{
  QPointF delta = gesture->centerPoint() - gesture->lastCenterPoint();
  if (delta.x() || delta.y()) {
    graphicsView->translate(delta.x(), delta.y());
  }

  double factor = gesture->scaleFactor();
  if (factor != 1.0) {
    graphicsView->scale(factor, factor);
  }
}
