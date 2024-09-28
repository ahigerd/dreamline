#include "mainwindow.h"
#include "editorview.h"
#include "tool.h"
#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QStyle>
#include <QActionGroup>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
  editor = new EditorView(this);
  setCentralWidget(editor);

  setMenuBar(new QMenuBar(this));
  makeFileMenu();
  makeToolMenu();

  fileNew();
}

MainWindow::~MainWindow() {
}

void MainWindow::makeFileMenu()
{
  QMenu* fileMenu = new QMenu(tr("&File"), this);
  menuBar()->addMenu(fileMenu);
  QAction* aNew = fileMenu->addAction(tr("&New"), this, SLOT(fileNew()), QStringLiteral("Ctrl+N"));
  QAction* aOpen = fileMenu->addAction(tr("&Open..."), this, SLOT(fileOpen()), QStringLiteral("Ctrl+O"));
  QAction* aSave = fileMenu->addAction(tr("&Save"), this, SLOT(fileSave()), QStringLiteral("Ctrl+S"));
  fileMenu->addAction(tr("Save &As..."), this, SLOT(fileSaveAs()), QStringLiteral("Ctrl+Shift+S"));
  fileMenu->addSeparator();
  fileMenu->addAction(tr("&Export..."), this, SLOT(fileExport()), QStringLiteral("Ctrl+E"));
  fileMenu->addSeparator();
  fileMenu->addAction(tr("E&xit"), qApp, SLOT(quit()));

  QToolBar* fileBar = new QToolBar(tr("&File"), this);
  addToolBar(Qt::TopToolBarArea, fileBar);
  aNew->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
  fileBar->addAction(aNew);
  aOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  fileBar->addAction(aOpen);
  aSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  fileBar->addAction(aSave);

  fileBar->setIconSize(QSize(16, 16));
}

void MainWindow::makeToolMenu()
{
  QActionGroup* toolGroup = new QActionGroup(this);

  QMenu* toolMenu = new QMenu(tr("&Tools"), this);
  menuBar()->addMenu(toolMenu);

  QToolBar* toolBar = new QToolBar(tr("&Tools"), this);
  addToolBar(Qt::LeftToolBarArea, toolBar);

  QAction* aVertex = Tool::makeAction(toolGroup, Tool::VertexTool);
  toolMenu->addAction(aVertex);
  toolBar->addAction(aVertex);

  QAction* aEdge = Tool::makeAction(toolGroup, Tool::EdgeTool);
  toolMenu->addAction(aEdge);
  toolBar->addAction(aEdge);

  QAction* aColor = Tool::makeAction(toolGroup, Tool::ColorTool);
  toolMenu->addAction(aColor);
  toolBar->addAction(aColor);

  QAction* aSplit = Tool::makeAction(toolGroup, Tool::SplitEdge);
  toolMenu->addAction(aSplit);
  toolBar->addAction(aSplit);

  aVertex->setChecked(true);

  QObject::connect(toolGroup, SIGNAL(triggered(QAction*)), editor, SLOT(setTool(QAction*)));
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
