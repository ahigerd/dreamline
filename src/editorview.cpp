#include "editorview.h"
#include "glviewport.h"
#include "polygonitem.h"
#include <QGraphicsScene>
#include <QGestureEvent>
#include <QPinchGesture>
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
  setScene(scene);
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
  double factor = gesture->scaleFactor();
  if (factor != 1.0) {
    scale(factor, factor);
  }

  QPointF delta = gesture->centerPoint() - gesture->lastCenterPoint();
  if (delta.x() || delta.y()) {
    translate(delta.x(), delta.y());
  }
}

void EditorView::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    setDragMode(RubberBandDrag);
  } else if (event->button() == Qt::MiddleButton) {
    setDragMode(ScrollHandDrag);
  } else {
    setDragMode(NoDrag);
  }
  QGraphicsView::mousePressEvent(event);
}

void EditorView::mouseReleaseEvent(QMouseEvent* event)
{
  QGraphicsView::mouseReleaseEvent(event);
  setDragMode(NoDrag);
}
