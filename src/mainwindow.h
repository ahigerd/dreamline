#ifndef DL_MAINWINDOW_H
#define DL_MAINWINDOW_H

#include <QMainWindow>
#include <QString>
class QPinchGesture;
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

  bool eventFilter(QObject* obj, QEvent* event);

private slots:
  void fileNew();
  void fileOpen();
  void fileSave();
  void fileSaveAs();
  void fileExport();

private:
  void makeFileMenu();

  void pinchGesture(QPinchGesture* gesture);

  QGraphicsView* graphicsView;
  GLViewport* viewport;
  QString savePath;
  QString exportPath;
};

#endif
