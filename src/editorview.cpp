#include "editorview.h"
#include "glviewport.h"
#include "dreamproject.h"
#include "tool.h"
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QScrollBar>
#include <QWheelEvent>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QPalette>
#include <QWindow>
#include <QColorDialog>
#include <QColor>
#include <QMenu>
#include <QGraphicsRectItem>
#include <cmath>

// #define ALT_RING_MODE 0
// #define ALT_RING_MODE 1
#define ALT_RING_MODE 2

EditorView::EditorView(QWidget* parent)
: QGraphicsView(parent), isPanning(false), isResizingRing(false), containsMouse(false), useRing(true),
  ringSize(20), currentTool(nullptr), underCursor(nullptr)
{
  glViewport = new GLViewport(this);
  glViewport->grabGesture(Qt::PinchGesture);
  setMouseTracking(true);
  glViewport->setMouseTracking(true);

  setViewport(glViewport);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setDragMode(NoDrag);
  setCursor(Qt::BlankCursor);
  setTool(Tool::VertexTool);
}

void EditorView::newProject()
{
  QGraphicsScene* oldScene = scene();

  project = new DreamProject(QSizeF(8.5, 11), this);
  setScene(project);

  underCursor = new QGraphicsRectItem(-3, -3, 6, 6);
  underCursor->setPen(QColor(Qt::transparent));
  underCursor->setFlag(QGraphicsItem::ItemIsMovable, true);
  underCursor->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  underCursor->setZValue(HUGE_VAL);
  project->addItem(underCursor);
  setCursorFromTool();

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
    if (currentTool) {
      bool handled = currentTool->mousePressEvent(this, event);
      if (handled) {
        return;
      }
    }
  } else if (event->button() == Qt::MiddleButton) {
    setDragMode(ScrollHandDrag);
    isPanning = true;
    dragStart = event->pos();
  } else if (event->button() == Qt::RightButton) {
    timer.start();
    setDragMode(NoDrag);
    isResizingRing = true;
    dragStart = lastDrag = event->globalPos();
    originalRingSize = ringSize;
    setCursor(Qt::BlankCursor);
  } else {
    setDragMode(NoDrag);
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
    QPoint pt = event->globalPos();
    int dx = pt.x() - lastDrag.x();
    int dy = pt.y() - lastDrag.y();
    int delta = std::sqrt(dx * dx + dy * dy);
    if (dx - dy < 0) {
      delta = -delta;
    }

    ringSize += delta;
    if (ringSize < 3) {
      ringSize = 3;
    }
    QCursor::setPos(dragStart);
    lastDrag = QCursor::pos();
#else
    ringSize = QLineF(dragStart, event->globalPos()).length();
#endif
  } else {
    bool handled = false;
    if (currentTool) {
      handled = currentTool->mouseMoveEvent(this, event);
    }
    if (!handled) {
      QGraphicsView::mouseMoveEvent(event);
    }
  }
  updateMouseRect();
}

void EditorView::mouseReleaseEvent(QMouseEvent* event)
{
  bool handled = false;
  if (event->button() == Qt::MiddleButton) {
    isPanning = false;
  } else if (event->button() == Qt::RightButton) {
    isResizingRing = false;
    updateMouseRect();
#if ALT_RING_MODE
    QCursor::setPos(dragStart);
#endif
    setCursorFromTool();
    if (timer.elapsed() < 250) {
      contextMenu(event->globalPos());
    }
  } else if (currentTool) {
    handled = currentTool->mouseReleaseEvent(this, event);
  }
  if (!handled) {
    QGraphicsView::mouseReleaseEvent(event);
  }
  setDragMode(NoDrag);
}

void EditorView::enterEvent(QEvent*)
{
  containsMouse = true;
  updateMouseRect();
}

void EditorView::leaveEvent(QEvent*)
{
  containsMouse = false;
  updateMouseRect();
}

void EditorView::wheelEvent(QWheelEvent* event)
{
  if (event->modifiers() & Qt::ControlModifier) {
    double factor = 1.0 + (event->angleDelta().y() / 1200.0);
    scale(factor, factor);
    return;
  }
  QGraphicsView::wheelEvent(event);
}

void EditorView::updateMouseRect()
{
  QPointF center = mapToScene(mapFromGlobal(QCursor::pos()));
  underCursor->setPos(center);
  QRectF mouseRect(center.x() - ringSize - 1.5, center.y() - ringSize - 1.5, 2 * ringSize + 3, 2 * ringSize + 3);
  updateScene({ mouseRect, lastMouseRect });
  lastMouseRect = mouseRect;
}

void EditorView::drawForeground(QPainter* p, const QRectF& rect)
{
  QGraphicsView::drawForeground(p, rect);
  if (isPanning || !containsMouse || !useRing) {
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

void EditorView::contextMenu(const QPoint& pos)
{
  QMenu menu;
  QAction* colorAction = menu.addAction(tr("Change Color..."));
  QAction* selected = menu.exec(pos);
  if (selected == colorAction) {
    selectColor();
    return;
  }
}

void EditorView::selectColor()
{
  QColor color = QColorDialog::getColor(lastColor, this, "Select Color");
  if (color.isValid()) {
    lastColor = color;
  }
}

void EditorView::setTool(QAction* toolAction)
{
  setTool(Tool::Type(toolAction->data().toInt()));
}

void EditorView::setTool(Tool::Type type)
{
  if (currentTool) {
    currentTool->deactivated(this);
  }
  qDebug() << "Selected:" << type;
  currentTool = Tool::get(type);
  if (currentTool) {
    currentTool->activated(this);
  }
  setCursorFromTool();
}

void EditorView::setCursorFromTool()
{
  if (currentTool) {
    Qt::CursorShape shape = currentTool->cursorShape();
    if (shape != Qt::BitmapCursor) {
      setCursor(shape);
      useRing = false;
      if (underCursor) {
        underCursor->hide();
        updateMouseRect();
      }
      return;
    }
  }
  setCursor(Qt::BlankCursor);
  useRing = true;
  if (underCursor) {
    underCursor->show();
    updateMouseRect();
  }
}

QList<QGraphicsItem*> EditorView::itemsInRing() const
{
  QPainterPath p;
  QPointF center = mapToScene(mapFromGlobal(QCursor::pos()));
  double scale = 1.0 / transform().m11();
  p.addEllipse(center, ringSize * scale, ringSize * scale);
  return scene()->items(p, Qt::IntersectsItemShape, Qt::DescendingOrder, transform());
}
