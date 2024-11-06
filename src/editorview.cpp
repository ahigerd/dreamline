#include "editorview.h"
#include "glviewport.h"
#include "gripitem.h"
#include "edgeitem.h"
#include "meshitem.h"
#include "tool.h"
#include "propertypanel.h"
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
#include <cmath>
#include <limits>

// #define ALT_RING_MODE 0
// #define ALT_RING_MODE 1
#define ALT_RING_MODE 2

OpenException::OpenException(const QString& what)
: std::runtime_error(what.toUtf8().constData())
{
  // initializers only
}

SaveException::SaveException(const QString& what)
: std::runtime_error(what.toUtf8().constData())
{
  // initializers only
}

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
  setRenderHint(QPainter::Antialiasing, true);

  m_colorAction = new QAction("Color", this);
  setColor(Qt::blue);
  QObject::connect(m_colorAction, SIGNAL(triggered()), this, SLOT(selectColor()));
}

void EditorView::newProject()
{
  currentTool->deactivated(this);
  QGraphicsScene* oldScene = scene();

  projectScene = new DreamProject(QSizeF(8.5, 11), this);
  projectScene->setCurrentEditor(this);
  setScene(projectScene);

  underCursor = new QGraphicsRectItem(-3, -3, 6, 6);
  underCursor->setPen(QColor(Qt::transparent));
  underCursor->setFlag(QGraphicsItem::ItemIsMovable, true);
  underCursor->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  underCursor->setZValue(HUGE_VAL);
  projectScene->addItem(underCursor);
  setCursorFromTool();

  delete oldScene;

  QObject::connect(projectScene, SIGNAL(projectModified(bool)), this, SIGNAL(projectModified(bool)));

  currentTool->activated(this);
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
  QPointF center = cursorPos();
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

QColor EditorView::color() const
{
  return lastColor;
}

void EditorView::setColor(const QColor& color)
{
  if (color != lastColor) {
    lastColor = color;
    QImage img(24, 24, QImage::Format_RGB32);
    img.fill(color.toRgb());
    QPainter p(&img);
    QPen pen(Qt::black, 0);
    pen.setCosmetic(true);
    p.setPen(pen);
    p.drawRect(0, 0, 23, 23);
    m_colorAction->setIcon(QPixmap::fromImage(img));
    emit colorSelected(color);
  }
}

QAction* EditorView::colorAction() const
{
  return m_colorAction;
}

void EditorView::selectColor()
{
  QColor color = QColorDialog::getColor(lastColor, this, "Select Color");
  if (color.isValid() && color != lastColor) {
    setColor(color);
  }
}

void EditorView::toggleSmooth()
{
  for (GripItem* grip : selectedItems<GripItem>()) {
    grip->setSmooth(!grip->isSmooth());
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

MeshItem* EditorView::activeMesh() const
{
  for (QGraphicsItem* item : items()) {
    MeshItem* mesh = dynamic_cast<MeshItem*>(item);
    if (!mesh) {
      continue;
    }
    GripItem* grip = mesh->activeVertex();
    if (grip) {
      return mesh;
    }
  }
  return nullptr;
}

GripItem* EditorView::activeVertex() const
{
  MeshItem* mesh = activeMesh();
  if (mesh) {
    return mesh->activeVertex();
  }
  return nullptr;
}

void EditorView::setActiveVertex(GripItem* vertex)
{
  MeshItem* targetMesh = vertex ? dynamic_cast<MeshItem*>(vertex->parentItem()) : nullptr;
  for (QGraphicsItem* item : items()) {
    MeshItem* mesh = dynamic_cast<MeshItem*>(item);
    if (!mesh) {
      continue;
    }
    if (mesh == targetMesh) {
      mesh->setActiveVertex(vertex);
    } else {
      mesh->setActiveVertex(nullptr);
    }
  }
  if (!targetMesh) {
    for (GripItem* item : projectScene->selectedItems<GripItem>()) {
      targetMesh = dynamic_cast<MeshItem*>(item->parentItem());
      if (targetMesh) {
        break;
      }
    }
  }
  if (targetMesh) {
    PropertyPanel* fill = targetMesh->fillPropertyPanel();
    PropertyPanel* stroke = targetMesh->strokePropertyPanel();
    emit propertyPanelChanged("fill", fill);
    emit propertyPanelChanged("stroke", stroke);
    if (fill) {
      fill->setCurrentMesh(targetMesh);
    }
    if (stroke) {
      stroke->setCurrentMesh(targetMesh);
    }
  } else {
    emit propertyPanelChanged("fill", nullptr);
    emit propertyPanelChanged("stroke", nullptr);
  }
  emit selectionChanged();
}

QPair<EdgeItem*, QPointF> EditorView::snapEdge() const
{
  QPointF mouse = cursorPos();
  EdgeItem* snapEdge = nullptr;
  QPointF snapPosition;
  QList<EdgeItem*> edges = itemsInRing<EdgeItem>();
  qreal closest = std::numeric_limits<qreal>::max();
  for (EdgeItem* edge : edges) {
    QPointF snap = edge->nearestPointOnLine(mouse);
    qreal distance = QLineF(mouse, snap).length();
    if (distance < closest) {
      snapPosition = snap;
      snapEdge = edge;
      closest = distance;
    }
  }
  return QPair<EdgeItem*, QPointF>(snapEdge, snapPosition);
}

GripItem* EditorView::snapGrip() const
{
  QPointF mouse = cursorPos();
  GripItem* snapGrip = nullptr;
  qreal closest = std::numeric_limits<qreal>::max();
  QList<GripItem*> grips = itemsInRing<GripItem>();
  for (GripItem* grip : grips) {
    qreal distance = QLineF(mouse, grip->pos()).length();
    if (distance < closest) {
      snapGrip = grip;
      closest = distance;
    }
  }
  return snapGrip;
}

QPointF EditorView::cursorPos() const
{
  return mapToScene(mapFromGlobal(QCursor::pos()));
}

bool EditorView::edgesVisible() const
{
  return m_edgesVisible && !m_preview;
}

void EditorView::setEdgesVisible(bool on)
{
  m_edgesVisible = on;
  updateScene({ mapToScene(rect()).boundingRect() });
}

bool EditorView::verticesVisible() const
{
  return m_verticesVisible && !m_preview;
}

void EditorView::setVerticesVisible(bool on)
{
  m_verticesVisible = on;
  updateScene({ mapToScene(rect()).boundingRect() });
}

bool EditorView::isPreview() const
{
  return m_preview;
}

void EditorView::setPreview(bool on)
{
  m_preview = on;
  updateScene({ mapToScene(rect()).boundingRect() });
}

DreamProject* EditorView::project() const
{
  return projectScene;
}
