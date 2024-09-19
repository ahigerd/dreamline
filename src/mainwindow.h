#ifndef DL_MAINWINDOW_H
#define DL_MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QString>


class MainWindow : public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  void openFile(const QString& path);
  void saveFile(const QString& path);
  ~MainWindow();

private slots:
  void fileNew();
  void fileOpen();
  void fileSave();
  void fileSaveAs();

private:
  QGraphicsView *graphicsView;
  QString savePath;

  void makeFileMenu();
};

#endif
