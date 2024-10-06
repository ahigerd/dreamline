#include "moveedge.h"
#include "editorview.h"
#include "edgeitem.h"
#include "gripitem.h"
#include "meshitem.h"
#include <QMouseEvent>

MoveEdgeTool::MoveEdgeTool()
: BaseTool()
{
}

void MoveEdgeTool::activated(EditorView* editor)
{
  editor->setVerticesVisible(false);
}

void MoveEdgeTool::deactivated(EditorView* editor)
{
  editor->setVerticesVisible(true);
  for (EdgeItem* item : editor->itemsOfType<EdgeItem>()) {
    item->hoverLeave();
  }
}

bool MoveEdgeTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  pressed = true;
  for (QGraphicsItem* item : editor->scene()->selectedItems()) {
    item->setSelected(false);
  }
  QList<EdgeItem*> gripsInRing = editor->itemsInRing<EdgeItem>();
  for (EdgeItem* item : gripsInRing)
  {
    item->leftGrip()->setSelected(true);
    item->rightGrip()->setSelected(true);
  }
  return false;
}

bool MoveEdgeTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  if (!pressed) {
    for (EdgeItem* item : editor->itemsOfType<EdgeItem>()) {
      item->hoverLeave();
    }
    QList<EdgeItem*> gripsInRing = editor->itemsInRing<EdgeItem>();
    for (EdgeItem* item : gripsInRing) {
      item->hoverEnter();
    }
  }
  return false;
}

bool MoveEdgeTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  pressed = false;
  for (EdgeItem* item : editor->itemsOfType<EdgeItem>()) {
    item->hoverLeave();
  }
  QList<EdgeItem*> gripsInRing = editor->itemsInRing<EdgeItem>();
  for (EdgeItem* item : gripsInRing) {
    item->hoverEnter();
  }
  return false;
}
