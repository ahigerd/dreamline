#include "mainwindow.h"
#include "editorview.h"
#include "tool.h"
#include "dreamproject.h"
#include "propertypanel.h"
#include "penstrokerenderer.h"
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
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
  editor = new EditorView(this);
  setCentralWidget(editor);
  QObject::connect(editor, SIGNAL(projectModified(bool)), this, SLOT(setWindowModified(bool)));
  QObject::connect(editor, SIGNAL(propertyPanelChanged(QString,PropertyPanel*)), this, SLOT(setPropertyPanel(QString,PropertyPanel*)));

  setMenuBar(new QMenuBar(this));
  makeFileMenu();
  makeToolMenu();
  updateRecentMenu();

  PenStrokePropertyPanel* placeholder = new PenStrokePropertyPanel;
  setPropertyPanel("fill", nullptr);
  setPropertyPanel("stroke", placeholder);
  placeholder->deleteLater();

  fileNew();

  QSettings settings;
  restoreGeometry(settings.value("window/geometry").toByteArray());
  restoreState(settings.value("window/state").toByteArray());
  // TODO: cascade window positions if other windows already exist
}

MainWindow::~MainWindow()
{
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
  fileBar->setObjectName("tb_file");
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
  toolBar->setObjectName("tb_tools");
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
  toolBar->addAction(editor->colorAction());

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
    // TODO: it might be nice to have a command to load the contents of another
    //       file into this one without deleting anything (e.g. shape library)
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
  QFileDialog dlg(this, tr("Export Dreamline File"), exportPath, tr("Dreamline files (*.dream);;All files (*)"));
  dlg.setDefaultSuffix("dream");
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  if (dlg.exec() == QDialog::Rejected) {
    return;
  }
  saveFile(dlg.selectedFiles().first());
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

  exportFile(dlg.selectedFiles().first(), dlg.selectedMimeTypeFilter());
}

void MainWindow::exportFile(const QString& path, const QString& format)
{
  exportPath = path;

  QByteArray formatCode = QImageWriter::imageFormatsForMimeType(format.toUtf8()).first();

  // TODO: configurable output DPI
  bool ok = editor->project()->exportToFile(path, formatCode.constData(), 100);

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

void MainWindow::setPropertyPanel(const QString& tag, PropertyPanel* panel)
{
  static QMap<QString, QString> labels{
    { "fill", tr("Fill Properties") },
    { "stroke", tr("Stroke Properties") },
  };
  QDockWidget* dock = docksByTag[tag];
  if (!dock) {
    docksByTag[tag] = dock = new QDockWidget(labels[tag], this);
    dock->setObjectName("dock_" + tag);
    dock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    // XXX: This feels extremely hacky
    resizeDocks({ dock }, { height() }, Qt::Vertical);
  }
  if (dock->widget()) {
    dock->widget()->hide();
  }
  dock->setWidget(panel);
  if (panel) {
    panel->show();
  }
}

void MainWindow::closeEvent(QCloseEvent*)
{
  QSettings settings;
  settings.setValue("window/geometry", saveGeometry());
  settings.setValue("window/state", saveState());
}
