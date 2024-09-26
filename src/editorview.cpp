#include "editorview.h"
#include "glviewport.h"
#include "dreamproject.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QScrollBar>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QPalette>
#include <QWindow>

EditorView::EditorView(QWidget* parent)
: QGraphicsView(parent), isPanning(false), isResizingRing(false), ringSize(10)
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
    setDragMode(RubberBandDrag);
  } else if (event->button() == Qt::MiddleButton) {
    setDragMode(ScrollHandDrag);
    isPanning = true;
    dragStart = event->pos();
  } else {
    setDragMode(NoDrag);
    isResizingRing = true;
    dragStart = event->globalPos();
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
    ringSize = QLineF(dragStart, event->globalPos()).length();
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
  }
  QGraphicsView::mouseReleaseEvent(event);
  setDragMode(NoDrag);
  updateMouseRect();
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
  QPointF center = mapFromGlobal(QCursor::pos());
  p->setRenderHint(QPainter::Antialiasing);
  p->setPen(QPen(Qt::black, 3.5));
  p->drawEllipse(center, ringSize, ringSize);
  p->setPen(QPen(Qt::white, 1.5));
  p->drawEllipse(center, ringSize, ringSize);
  /*
  if (isResizingRing) {
    QPen pen(Qt::black, 0);
    pen.setCosmetic(true);
    p->setPen(pen);
    p->drawLine(center, mapFromGlobal(dragStart));
  }
  */
}
