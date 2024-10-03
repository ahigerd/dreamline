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
  GripItem* grip = closestGrip;
  MeshItem* mesh = editor->activeMesh();
  GripItem* oldActive = editor->activeVertex();
  if (closestEdge) {
    if (!closestGrip) {
      closestEdge->split(snapPoint);
    }
    else {
      editor->setActiveVertex(closestGrip);
    }
    if (oldActive) {
      mesh->splitPolygon(oldActive, editor->activeVertex());
    }
  }
  /* if (grip) { */
  /*   if (mesh && (event->modifiers() & Qt::ShiftModifier) && mesh->activeVertex() != grip) { */
  /*     mesh->splitPolygon(mesh->activeVertex(), grip); */
  /*     return true; */
  /*   } else { */
  /*     editor->setActiveVertex(grip); */
  /*   } */
  /* } else { */
  /*   editor->setActiveVertex(nullptr); */
  /* } */
  return false;
}

bool SplitEdgeTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  if (marker == nullptr) {
    marker = new MarkerItem();
    editor->scene()->addItem(marker);
  }
  closestEdge = nullptr;
  closestGrip = nullptr;
  QPointF mouse = editor->mapToScene(event->pos());
  QList<EdgeItem*> edges = editor->itemsInRing<EdgeItem>();
  qreal closest = std::numeric_limits<qreal>::max();
  for (EdgeItem* edge : edges) {
    QPointF snap = edge->nearestPointOnLine(mouse);
    qreal distance = QLineF(mouse, snap).length();
    if (distance < closest) {
      snapPoint = snap;
      closestEdge = edge;
      closest = distance;
    }
  }
  closest = std::numeric_limits<qreal>::max();
  QList<GripItem*> grips = editor->itemsInRing<GripItem>();
  for (GripItem* grip : grips) {
    qreal distance = QLineF(mouse, grip->pos()).length();
    if (distance < closest) {
      snapPoint = grip->pos();
      closestGrip = grip;
      closest = distance;
    }
  }
  if (closestEdge || closestGrip) {
    marker->setVisible(true);
    marker->setBrush(editor->lastColor);
    marker->setPos(snapPoint);
  }
  else {
    marker->setVisible(false);
    marker->setPos(mouse);
  }
  return false;
}

bool SplitEdgeTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}
