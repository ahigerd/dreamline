#include "splitedge.h"
#include "editorview.h"
#include "meshitem.h"
#include "gripitem.h"
#include <QMouseEvent>
#include <QDebug>
#include <limits>

SplitEdgeTool::SplitEdgeTool()
: BaseTool()
{
}

bool SplitEdgeTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  GripItem* grip = dynamic_cast<GripItem*>(editor->itemAt(event->pos()));
  MeshItem* mesh = editor->activeMesh();
  if (grip) {
    if (mesh && (event->modifiers() & Qt::ShiftModifier) && mesh->activeVertex() != grip) {
      mesh->splitPolygon(mesh->activeVertex(), grip);
      return true;
    } else {
      editor->setActiveVertex(grip);
    }
  } else {
    editor->setActiveVertex(nullptr);
  }
  return false;
}

bool SplitEdgeTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  if (marker == nullptr) {
    marker = new MarkerItem();
    editor->scene()->addItem(marker);
    editor->setCursor(Qt::ArrowCursor);
  }
  QPointF mouse = editor->mapToScene(event->pos());
  QList<EdgeItem*> items = editor->itemsInRing<EdgeItem>();
  qreal closest = std::numeric_limits<qreal>::max();
  for (EdgeItem* item : items) {
    QPointF snap = item->nearestPointOnLine(mouse);
    qreal distance = QLineF(mouse, snap).length();
    if (distance < closest) {
      snapPoint = snap;
      closestEdge = item;
      closest = distance;
    }
  }
  if (closestEdge) {
      qDebug() << "test";
    marker->setPos(snapPoint);
  }
  else {
    marker->setPos(mouse);
  }
  return false;
}

bool SplitEdgeTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}
