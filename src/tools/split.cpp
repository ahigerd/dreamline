#include "split.h"
#include "editorview.h"
#include "meshitem.h"
#include "gripitem.h"
#include <QMouseEvent>
#include <QDebug>
#include <limits>

SplitTool::SplitTool()
: BaseTool()
{
}

void SplitTool::activated(EditorView* editor)
{
  if (marker == nullptr) {
    marker = new MarkerItem();
    editor->scene()->addItem(marker);
    marker->setHighlight(QColor("#FF4444"));
  }
  GripItem* closestGrip = editor->snapGrip();
  EdgeItem* closestEdge = editor->snapEdge();

  if (closestEdge || closestGrip) {
    marker->setVisible(true);
    if (closestGrip) {
      marker->setBrush(closestGrip->color());
    }
    else {
      marker->setBrush(closestEdge->colorAt(editor->snapPosition()));
    }
    marker->setPos(editor->snapPosition());
  }
  else {
    marker->setVisible(false);
  }
}

void SplitTool::deactivated(EditorView* editor)
{
    marker->setVisible(false);
}

bool SplitTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  MeshItem* mesh = editor->activeMesh();
  GripItem* oldActive = editor->activeVertex();
  GripItem* closestGrip = editor->snapGrip();
  EdgeItem* closestEdge = editor->snapEdge();
  if (closestEdge) {
    if (!closestGrip) {
      closestEdge->split(editor->snapPosition());
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

bool SplitTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  if (marker == nullptr) {
    marker = new MarkerItem();
    editor->scene()->addItem(marker);
    marker->setHighlight(QColor("#FF4444"));
  }
  GripItem* closestGrip = editor->snapGrip();
  EdgeItem* closestEdge = editor->snapEdge();

  if (closestEdge || closestGrip) {
    marker->setVisible(true);
    if (closestGrip) {
      marker->setBrush(closestGrip->color());
    }
    else {
      marker->setBrush(closestEdge->colorAt(editor->snapPosition()));
    }
    marker->setPos(editor->snapPosition());
  }
  else {
    marker->setVisible(false);
  }
  return false;
}

bool SplitTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}
