#include "mainwindow.h"
#include "editorview.h"
#include "tool.h"
#include "dreamproject.h"
#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QStyle>
#include <QActionGroup>
#include <QMessageBox>
#include <QFileInfo>
#include <QSettings>
#include <QPainter>
#include <QImageWriter>
#include <QMimeDatabase>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
  editor = new EditorView(this);
  setCentralWidget(editor);
  QObject::connect(editor, SIGNAL(projectModified(bool)), this, SLOT(setWindowModified(bool)));
  QObject::connect(editor, SIGNAL(colorSelected(QColor)), this, SLOT(setCurrentColor(QColor)));

  setMenuBar(new QMenuBar(this));
  makeFileMenu();
  makeToolMenu();
  updateRecentMenu();

  fileNew();
}

MainWindow::~MainWindow() {
}

void MainWindow::makeFileMenu()
{
  recentMenu = new QMenu(tr("Open &Recent"), this);
  QObject::connect(recentMenu, SIGNAL(triggered(QAction*)), this, SLOT(fileOpenRecent(QAction*)));

  QMenu* fileMenu = new QMenu(tr("&File"), this);
  menuBar()->addMenu(fileMenu);
  QAction* aNew = fileMenu->addAction(tr("&New"), this, SLOT(fileNew()), QStringLiteral("Ctrl+N"));
  QAction* aOpen = fileMenu->addAction(tr("&Open..."), this, SLOT(fileOpen()), QStringLiteral("Ctrl+O"));
  fileMenu->addMenu(recentMenu);
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

  QAction* aPreview = fileBar->addAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), tr("Preview"));
  aPreview->setCheckable(true);
  QObject::connect(aPreview, SIGNAL(toggled(bool)), editor, SLOT(setPreview(bool)));

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

  QAction* aSplit = Tool::makeAction(toolGroup, Tool::Split);
  toolMenu->addAction(aSplit);
  toolBar->addAction(aSplit);

  toolBar->addSeparator();
  colorButton = toolBar->addAction("Color", editor, SLOT(selectColor()));
  setCurrentColor(editor->lastColor);

  toolBar->addSeparator();
  QAction* aSmooth = toolBar->addAction("Sharp/Smooth", editor, SLOT(toggleSmooth()));
  aSmooth->setShortcut(QKeySequence("t"));

  aVertex->setChecked(true);

  QObject::connect(toolGroup, SIGNAL(triggered(QAction*)), editor, SLOT(setTool(QAction*)));
}

void MainWindow::fileNew()
{
  savePath.clear();
  editor->newProject();
  setWindowModified(false);
  updateTitle();
}

void MainWindow::fileOpen()
{
  QString path = QFileDialog::getOpenFileName(this, tr("Open Dreamline File"), QString(), tr("Dreamline Files (*.dream)"));
  if (path.isEmpty()) {
    return;
  }
  openFile(path);
}

void MainWindow::fileOpenRecent(QAction* action)
{
  openFile(action->data().toString());
}

void MainWindow::openFile(const QString& path)
{
  savePath = path;

  try {
    editor->newProject();
    editor->project()->open(path);
    setWindowModified(false);
    updateTitle();
    addToRecent(path);
  } catch (OpenException& err) {
    QMessageBox::warning(this, tr("Error loading Dreamline file"), QString::fromUtf8(err.what()));
    fileNew();
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
  // TODO: This is slightly hacky and wouldn't work on mobile or web.
  // The correct solution is to not use the static convenience function,
  // and call setDefaultSuffix().
  QFileInfo info(path);
  if (!info.exists() && info.suffix().isEmpty()) {
    path += ".dream";
  }
  saveFile(path);
}

void MainWindow::saveFile(const QString& path)
{
  savePath = path;
  updateTitle();
  try {
    editor->project()->save(path);
    setWindowModified(false);
    addToRecent(path);
  } catch (SaveException& err) {
    QMessageBox::warning(this, tr("Error saving Dreamline file"), QString::fromUtf8(err.what()));
  }
}

void MainWindow::fileExport()
{
  QFileDialog dlg(this, tr("Export Dreamline File"), exportPath);
  QStringList filters;
  for (const QByteArray& mime : QImageWriter::supportedMimeTypes()) {
    filters << QString::fromUtf8(mime);
  }
  dlg.setMimeTypeFilters(filters);
  dlg.selectMimeTypeFilter("image/png");
  dlg.setDefaultSuffix("png");
  dlg.setAcceptMode(QFileDialog::AcceptSave);

  // I don't know why QFileDialog doesn't do this by default,
  // but this is the recommended solution.
  QObject::connect(&dlg, &QFileDialog::filterSelected, [&dlg]{
    dlg.setDefaultSuffix(QMimeDatabase().mimeTypeForName(dlg.selectedMimeTypeFilter()).preferredSuffix());
  });

  if (dlg.exec() == QDialog::Rejected) {
    return;
  }

  exportFile(dlg.selectedFiles().first());
}

void MainWindow::exportFile(const QString& path)
{
  exportPath = path;

  bool ok = editor->project()->exportToFile(path, "PNG", 200);

  if (!ok) {
    QMessageBox::warning(this, tr("Error exporting Dreamline file"), tr("%1 could not be saved.").arg(path));
  }
}

void MainWindow::updateTitle()
{
  QString name;
  if (!savePath.isEmpty()) {
    QFileInfo info(savePath);
    name = info.fileName();
  }
  if (name.isEmpty()) {
    name = tr("Untitled");
  }
  setWindowTitle(tr("%1[*] - Dreamline").arg(name));
  setWindowFilePath(savePath);
}

void MainWindow::updateRecentMenu()
{
  QSettings settings;
  recentMenu->clear();

  int i = 0;
  for (const QString& path : settings.value("recents").toStringList()) {
    ++i;
    QFileInfo info(path);
    QString name = info.fileName();
    QAction* action = recentMenu->addAction(tr("[&%1] %2").arg(i).arg(name));
    action->setData(name);
  }
}

void MainWindow::addToRecent(const QString& path)
{
  QSettings settings;
  QStringList recents = settings.value("recents").toStringList();
  recents.removeAll(path);
  recents.insert(0, path);
  recents = recents.mid(0, 10);
  settings.setValue("recents", recents);

  updateRecentMenu();
}

QColor MainWindow::currentColor() const
{
  return editor->lastColor;
}

void MainWindow::setCurrentColor(const QColor& color)
{
  QImage img(24, 24, QImage::Format_RGB32);
  img.fill(color.toRgb());
  QPainter p(&img);
  QPen pen(Qt::black, 0);
  pen.setCosmetic(true);
  p.setPen(pen);
  p.drawRect(0, 0, 23, 23);
  colorButton->setIcon(QPixmap::fromImage(img));
  editor->lastColor = color;
}
