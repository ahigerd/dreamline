#ifndef DL_EDITORVIEW_H
#define DL_EDITORVIEW_H

#include <QGraphicsView>
class QPinchGesture;
class GLViewport;

class EditorView : public QGraphicsView
{
Q_OBJECT
public:
  EditorView(QWidget* parent = nullptr);

  void newProject();
  // void openProject(const QString& path);
  // void saveProject(const QString& path);

protected:
  bool viewportEvent(QEvent* event);

private:
  void pinchGesture(QPinchGesture* gesture);

  GLViewport* glViewport;
};

#endif
