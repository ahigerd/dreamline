#ifndef DL_EDITORVIEW_H
#define DL_EDITORVIEW_H

#include <QMouseEvent>
#include <QGraphicsView>
#include <QElapsedTimer>
class DreamProject;
class QPinchGesture;
class GLViewport;
class GripItem;

class EditorView : public QGraphicsView
{
Q_OBJECT
public:
  EditorView(QWidget* parent = nullptr);

  void newProject();
  // void openProject(const QString& path);
  // void saveProject(const QString& path);

  QList<GripItem*> getSelectedVertices() const;
  QList<GripItem*> verticesInRing() const;

protected:
  bool viewportEvent(QEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void drawForeground(QPainter* p, const QRectF& rect);

private:
  void pinchGesture(QPinchGesture* gesture);
  void updateMouseRect();

  void contextMenu(const QPoint& pos);
  void selectColor();

  QElapsedTimer timer;
  GLViewport* glViewport;
  DreamProject* project;
  bool isPanning, isResizingRing;
  float ringSize, originalRingSize;
  QPoint dragStart;
  QRectF lastMouseRect;
};

#endif
