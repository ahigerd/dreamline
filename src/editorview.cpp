#include "editorview.h"
#include "glviewport.h"
#include "dreamproject.h"
#include "gripitem.h"
#include "tool.h"
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QScrollBar>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QPalette>
#include <QWindow>
#include <cmath>

// #define ALT_RING_MODE 0
// #define ALT_RING_MODE 1
#define ALT_RING_MODE 2

EditorView::EditorView(QWidget* parent)
: QGraphicsView(parent), isPanning(false), isResizingRing(false), ringSize(10), currentTool(nullptr)
{
  glViewport = new GLViewport(this);
  glViewport->grabGesture(Qt::PinchGesture);
  setMouseTracking(true);
  glViewport->setMouseTracking(true);

  setViewport(glViewport);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setDragMode(NoDrag);
}

void EditorView::newProject()
{
  QGraphicsScene* oldScene = scene();

  project = new DreamProject(QSizeF(8.5, 11), this);
  setScene(project);

  delete oldScene;
}

bool EditorView::viewportEvent(QEvent* event)
{
  if (event->type() == QEvent::Gesture) {
    QGestureEvent* gesture = static_cast<QGestureEvent*>(event);
    QPinchGesture* pinch = static_cast<QPinchGesture*>(gesture->gesture(Qt::PinchGesture));
    if (pinch) {
      pinchGesture(pinch);
      return true;
    }
  }
  return QGraphicsView::viewportEvent(event);
}

void EditorView::pinchGesture(QPinchGesture* gesture)
{
  // The first touch might have started a drag. Cancel it if so.
  setDragMode(NoDrag);

  QPointF delta = gesture->centerPoint() - gesture->lastCenterPoint();
  if (delta.x() || delta.y()) {
    translate(delta.x(), delta.y());
  }

  double factor = gesture->scaleFactor();
  if (factor != 1.0) {
    scale(factor, factor);
  }
}

void EditorView::mousePressEvent(QMouseEvent* event)
{
  if (isPanning) {
    // While using a middle-drag, don't process other clicks
    return;
  }
  if (event->button() == Qt::LeftButton) {
    for (GripItem* grip : getSelectedVertices()) {
      grip->setSelected(false);
    }
    QList<GripItem*> gripsInRing = verticesInRing();
    for (GripItem* item : gripsInRing)
    {
      item->setSelected(true);
    }
  } else if (event->button() == Qt::MiddleButton) {
    setDragMode(ScrollHandDrag);
    isPanning = true;
    dragStart = event->pos();
  } else {
    setDragMode(NoDrag);
    isResizingRing = true;
    dragStart = event->globalPos();
    originalRingSize = ringSize;
    setCursor(Qt::BlankCursor);
  }
  QGraphicsView::mousePressEvent(event);
}

void EditorView::mouseMoveEvent(QMouseEvent* event)
{
  if (isPanning) {
    QPoint delta = dragStart - event->pos();
    if (!delta.isNull()) {
      horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
      verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());
      dragStart = event->pos();
    }
  } else if (isResizingRing) {
#if ALT_RING_MODE == 2
    int delta = event->globalPos().x() - dragStart.x();
    ringSize = originalRingSize + delta;
    if (ringSize < 3) {
      originalRingSize = 3 - delta;
      ringSize = 3;
    }
#else
    ringSize = QLineF(dragStart, event->globalPos()).length();
#endif
  } else {
    QGraphicsView::mouseMoveEvent(event);
  }
  updateMouseRect();
}

void EditorView::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::MiddleButton) {
    isPanning = false;
  } else if (event->button() == Qt::RightButton) {
    isResizingRing = false;
    updateMouseRect();
#if ALT_RING_MODE
    QCursor::setPos(dragStart);
#endif
    unsetCursor();
  }
  QGraphicsView::mouseReleaseEvent(event);
  setDragMode(NoDrag);
}

void EditorView::updateMouseRect()
{
  QPointF center = mapToScene(mapFromGlobal(QCursor::pos()));
  QRectF mouseRect(center.x() - ringSize - 1.5, center.y() - ringSize - 1.5, 2 * ringSize + 3, 2 * ringSize + 3);
  updateScene({ mouseRect, lastMouseRect });
  lastMouseRect = mouseRect;
}

void EditorView::drawForeground(QPainter* p, const QRectF& rect)
{
  QGraphicsView::drawForeground(p, rect);
  if (isPanning) {
    return;
  }
  p->resetTransform();
#if ALT_RING_MODE
  QPointF center = mapFromGlobal(isResizingRing ? dragStart : QCursor::pos());
#else
  QPointF center = mapFromGlobal(QCursor::pos());
#endif
  p->setRenderHint(QPainter::Antialiasing);
  p->setPen(QPen(Qt::black, 4));
  p->drawEllipse(center, ringSize, ringSize);
  p->setPen(QPen(Qt::white, 2));
  p->drawEllipse(center, ringSize, ringSize);
  p->setPen(QPen(QColor(128, 128, 128), 1.5));
  p->drawEllipse(center + QPointF(0.5, 0.5), ringSize, ringSize);
  p->setPen(QPen(Qt::white, 1));
  p->drawEllipse(center - QPointF(0.5, 0.5), ringSize, ringSize);
  /*
  if (isResizingRing) {
    QPen pen(Qt::black, 0);
    pen.setCosmetic(true);
    p->setPen(pen);
    p->drawLine(center, mapFromGlobal(dragStart));
  }
  */
}

QList<GripItem*> EditorView::getSelectedVertices() const
{
  auto items = scene()->selectedItems();
  QList<GripItem*> grips;
  for (QGraphicsItem* item : items) {
    GripItem* grip = dynamic_cast<GripItem*>(item);
    if (grip) {
      grips << grip;
    }
  }
  return grips;
}

QList<GripItem*> EditorView::verticesInRing() const
{
  QPainterPath p;
  QPointF center = mapToScene(mapFromGlobal(QCursor::pos()));
  double scale = 1.0 / transform().m11();
  p.addEllipse(center, ringSize * scale, ringSize * scale);

  auto items = scene()->items(p, Qt::IntersectsItemShape, Qt::DescendingOrder, transform());
  QList<GripItem*> grips;
  for (QGraphicsItem* item : items) {
    GripItem* grip = dynamic_cast<GripItem*>(item);
    if (grip) {
      grips << grip;
    }
  }
  return grips;
}

void EditorView::setTool(QAction* toolAction)
{
  Tool::Type type = Tool::Type(toolAction->data().toInt());
  qDebug() << "Selected:" << type;
  currentTool = Tool::get(type);
}
