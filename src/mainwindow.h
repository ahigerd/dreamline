#ifndef DL_MAINWINDOW_H
#define DL_MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QUndoCommand>
#include <QUndoView>
class QGraphicsView;
class GLViewport;

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void openFile(const QString& path);
  void saveFile(const QString& path);
  void exportFile(const QString& path);

private slots:
  void fileNew();
  void fileOpen();
  void fileSave();
  void fileSaveAs();
  void fileExport();

  void deleteItem();

private:
  void makeFileMenu();
  void makeEditMenu();

  void createActions();
  void createUndoView();

  QGraphicsView* graphicsView;
  GLViewport* viewport;
  QString savePath;
  QString exportPath;

  QAction* deleteAction;
  QAction* undoAction;
  QAction* redoAction;

  QUndoStack* undoStack;
  QUndoView* undoView;
};

#endif
