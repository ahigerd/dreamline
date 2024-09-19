#include "mainwindow.h"
#include "polygonitem.h"
#include <QApplication>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), graphicsView(new QGraphicsView(this))
{
  setMenuBar(new QMenuBar(this));
  makeFileMenu();
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

void MainWindow::makeFileMenu()
{
  QMenu* fileMenu = new QMenu(tr("&File"), this);
  menuBar()->addMenu(fileMenu);
  fileMenu->addAction(tr("&New"), this, SLOT(fileNew()), QStringLiteral("Ctrl+N"));
  fileMenu->addAction(tr("&Open..."), this, SLOT(fileOpen()), QStringLiteral("Ctrl+O"));
  fileMenu->addAction(tr("&Save"), this, SLOT(fileSave()), QStringLiteral("Ctrl+S"));
  fileMenu->addAction(tr("Save &As..."), this, SLOT(fileSaveAs()), QStringLiteral("Ctrl+Shift+S"));
  fileMenu->addSeparator();
  fileMenu->addAction(tr("E&xit"), qApp, SLOT(quit()));
}

void MainWindow::fileNew()
{
  savePath.clear();
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
