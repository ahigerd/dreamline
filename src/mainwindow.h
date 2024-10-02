#ifndef DL_MAINWINDOW_H
#define DL_MAINWINDOW_H

#include <QMainWindow>
#include <QString>
class EditorView;

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

private:
  void makeFileMenu();
  void makeToolMenu();
  void updateTitle();

  EditorView* editor;
  QString savePath;
  QString exportPath;
};

#endif
