#include "mainwindow.h"
#include "polygonitem.h"
#include "glviewport.h"
#include "qundostack.h"
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QUndoCommand>
#include <QUndoView>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), graphicsView(new QGraphicsView(this))
{
  undoStack = new QUndoStack(this);

  setMenuBar(new QMenuBar(this));
  makeFileMenu();
  makeEditMenu();


  /* createUndoView(); */


  viewport = new GLViewport(graphicsView);
  graphicsView->setViewport(viewport);

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

void MainWindow::makeEditMenu()
{
  QMenu* editMenu = new QMenu(tr("&Edit"), this);
  menuBar()->addMenu(editMenu);

  undoAction = undoStack->createUndoAction(this, tr("&Undo"));
  undoAction->setShortcuts(QKeySequence::Undo);
  redoAction = undoStack->createRedoAction(this, tr("&Redo"));
  redoAction->setShortcuts(QKeySequence::Redo);
  deleteAction = new QAction(tr("&Delete Item"), this);
  deleteAction->setShortcut(tr("Del"));
  connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteItem);

  editMenu->addAction(undoAction);
  editMenu->addAction(redoAction);
  editMenu->addSeparator();
  editMenu->addAction(deleteAction);
}

void MainWindow::createUndoView()
{
  undoView = new QUndoView(undoStack);
  undoView->setWindowTitle(tr("Command List"));
  undoView->show();
  undoView->setAttribute(Qt::WA_QuitOnClose, false);
}

void MainWindow::deleteItem()
{
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
