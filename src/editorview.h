#ifndef DL_EDITORVIEW_H
#define DL_EDITORVIEW_H

#include <QGraphicsView>
class DreamProject;
class QPinchGesture;
class GLViewport;
class GripItem;
class Tool;

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

public slots:
  void setTool(QAction* toolAction);

protected:
  bool viewportEvent(QEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void drawForeground(QPainter* p, const QRectF& rect);

private:
  void pinchGesture(QPinchGesture* gesture);
  void updateMouseRect();

  GLViewport* glViewport;
  DreamProject* project;
  bool isPanning, isResizingRing;
  float ringSize, originalRingSize;
  QPoint dragStart;
  QRectF lastMouseRect;
  Tool* currentTool;
};

#endif
