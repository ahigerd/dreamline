#ifndef DL_EDITORVIEW_H
#define DL_EDITORVIEW_H

#include <QMouseEvent>
#include <QGraphicsView>
#include <QElapsedTimer>
#include <QPainterPath>
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

  QColor lastColor = Qt::blue;

template <typename ItemType>
QList<ItemType*> getSelectedItems() const
{
  auto items = scene()->selectedItems();
  QList<ItemType*> grips;
  for (QGraphicsItem* item : items) {
    ItemType* grip = dynamic_cast<ItemType*>(item);
    if (grip) {
      grips << grip;
    }
  }
  return grips;
}

template <typename ItemType>
QList<ItemType*> itemsInRing() const
{
  QPainterPath p;
  QPointF center = mapToScene(mapFromGlobal(QCursor::pos()));
  double scale = 1.0 / transform().m11();
  p.addEllipse(center, ringSize * scale, ringSize * scale);

  auto items = scene()->items(p, Qt::IntersectsItemShape, Qt::DescendingOrder, transform());
  QList<ItemType*> grips;
  for (QGraphicsItem* item : items) {
    ItemType* grip = dynamic_cast<ItemType*>(item);
    if (grip) {
      grips << grip;
    }
  }
  return grips;
}



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
  Tool* currentTool;
  QGraphicsRectItem* underCursor;
};

#endif
