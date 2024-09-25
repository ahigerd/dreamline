#include "mainwindow.h"
#include "editorview.h"
#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
  setMenuBar(new QMenuBar(this));
  makeFileMenu();

  editor = new EditorView(this);
  setCentralWidget(editor);

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
  editor->newProject();
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

  // TODO: editor->openProject(path);
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
  // TODO: bool ok = editor->saveProject(path);
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
