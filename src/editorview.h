#ifndef DL_EDITORVIEW_H
#define DL_EDITORVIEW_H

#include <QMouseEvent>
#include <QGraphicsView>
#include <QElapsedTimer>
#include "tool.h"
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

public slots:
  void setTool(QAction* toolAction);
  void setTool(Tool::Type type);

protected:
  bool viewportEvent(QEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void wheelEvent(QWheelEvent* event);
  void drawForeground(QPainter* p, const QRectF& rect);

private:
  void pinchGesture(QPinchGesture* gesture);
  void updateMouseRect();
  void setCursorFromTool();

  void contextMenu(const QPoint& pos);
  void selectColor();

  QElapsedTimer timer;
  GLViewport* glViewport;
  DreamProject* project;
  bool isPanning, isResizingRing, containsMouse, useRing;
  float ringSize, originalRingSize;
  QPoint dragStart, lastDrag;
  QRectF lastMouseRect;
  QColor lastColor = Qt::blue;
  Tool* currentTool;
  QGraphicsRectItem* underCursor;
};

#endif
