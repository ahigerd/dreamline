#include "editorview.h"
#include "glviewport.h"
#include "polygonitem.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QScrollBar>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QPalette>
#include <QWindow>

EditorView::EditorView(QWidget* parent)
: QGraphicsView(parent)
{
  glViewport = new GLViewport(this);
  glViewport->grabGesture(Qt::PinchGesture);

  setViewport(glViewport);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void EditorView::newProject()
{
  QGraphicsScene* oldScene = scene();

  QGraphicsScene* scene = new QGraphicsScene(this);
  // 8.5x11 + 1" margins on all sides @ 100dpi
  scene->setSceneRect(-650, -525, 1300, 1050);
  scene->setBackgroundBrush(palette().brush(QPalette::Window));
  setScene(scene);

  pageItem = new QGraphicsRectItem(QRectF(-550, -425, 1100, 850));
  pageItem->setBrush(palette().brush(QPalette::Base));
  pageItem->setPen(QPen(palette().color(QPalette::WindowText), 0));
  pageItem->setZValue(-1);
  scene->addItem(pageItem);

  PolygonItem* p = new PolygonItem;
  scene->addItem(p);

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
    panStart = event->pos();
  } else {
    setDragMode(NoDrag);
  }
  QGraphicsView::mousePressEvent(event);
}

void EditorView::mouseMoveEvent(QMouseEvent* event)
{
  if (isPanning) {
    QPoint delta = panStart - event->pos();
    if (!delta.isNull()) {
      horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
      verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());
      panStart = event->pos();
    }
  } else {
    QGraphicsView::mouseMoveEvent(event);
  }
}

void EditorView::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::MiddleButton) {
    isPanning = false;
  }
  QGraphicsView::mouseReleaseEvent(event);
  setDragMode(NoDrag);
}
