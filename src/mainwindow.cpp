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
  QString path = QFileDialog::getOpenFileName(this, tr("Open Dreamline File"), QString(), tr("Dreamline Files (*.dream)"));
  if (path.isEmpty()) {
    return;
  }
  openFile(path);
}

void MainWindow::openFile(const QString& path)
{
  savePath = path;

  try {
    editor->openProject(path);
  } catch (OpenException& err) {
    QMessageBox::warning(this, tr("Error loading DreamLine file"), QString::fromUtf8(err.what()));
  }
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
  QString path = QFileDialog::getSaveFileName(this, tr("Save Dreamline File"), savePath, tr("Dreamline Files (*.dream)"));
  if (path.isEmpty()) {
    return;
  }
  saveFile(path);
}

void MainWindow::saveFile(const QString& path)
{
  savePath = path;
  try {
    editor->saveProject(path);
  } catch (SaveException& err) {
    QMessageBox::warning(this, tr("Error saving Dreamline file"), QString::fromUtf8(err.what()));
  }
}

void MainWindow::fileExport()
{
  QString path = QFileDialog::getSaveFileName(this, tr("Export Dreamline File"), exportPath, tr("PNG Files (*.png)"));
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
    QMessageBox::warning(this, tr("Error exporting Dreamline file"), tr("%1 could not be saved.").arg(path));
  }
}
