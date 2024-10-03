#include "splitedge.h"
#include "editorview.h"
#include "meshitem.h"
#include "gripitem.h"
#include <QMouseEvent>
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
  if (!marker) {
    marker = new MarkerItem();
    editor->scene()->addItem(marker);
  }
  QList<EdgeItem*> items = editor->itemsInRing<EdgeItem>();
  qreal closest = std::numeric_limits<qreal>::max();
  for (EdgeItem* item : items) {
    QPointF snap = item->nearestPointOnLine(editor->pos());
    qreal distance = QLineF(editor->pos(), snap).length();
    if (distance < closest) {
      snapPoint = snap;
      closestEdge = item;
    }
  }
  if (!closestEdge) {
    marker->setPos(snapPoint);
  }
  return false;
}

bool SplitEdgeTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}
