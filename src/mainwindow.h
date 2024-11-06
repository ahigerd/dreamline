#ifndef DL_MAINWINDOW_H
#define DL_MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
class QMenu;
class EditorView;
class PropertyPanel;

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void openFile(const QString& path);
  void saveFile(const QString& path);
  void exportFile(const QString& path, const QString& format = "image/png");

private slots:
  void fileNew();
  void fileOpen();
  void fileOpenRecent(QAction* action);
  void fileSave();
  void fileSaveAs();
  void fileExport();

  void setPropertyPanel(const QString& tag, PropertyPanel* panel);

private:
  void makeFileMenu();
  void makeToolMenu();
  void updateTitle();

  void updateRecentMenu();
  void addToRecent(const QString& path);

  EditorView* editor;
  QAction* colorButton;
  QMenu* recentMenu;
  QString savePath;
  QString exportPath;
  QMap<QString, QDockWidget*> docksByTag;
};

#endif
