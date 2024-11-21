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
  // Hide grips when the edge tool is selected
  editor->setVerticesVisible(false);
  // Start highlighting edges when tool is activated
  for (EdgeItem* item : editor->itemsInRing<EdgeItem>()) {
    item->hoverEnter();
  }
}

void MoveEdgeTool::deactivated(EditorView* editor)
{
  // Restore grip visibility when switching to a different tool
  editor->setVerticesVisible(true);
  // Reset edge highlighting when switching to a different tool
  for (EdgeItem* item : editor->itemsOfType<EdgeItem>()) {
    item->hoverLeave();
  }
}

bool MoveEdgeTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  // Clear selection
  for (QGraphicsItem* item : editor->scene()->selectedItems()) {
    item->setSelected(false);
  }
  // Select all grips connected to edges within the tool ring
  for (EdgeItem* item : editor->itemsInRing<EdgeItem>()) {
    item->leftGrip()->setSelected(true);
    item->rightGrip()->setSelected(true);
  }
  return false;
}

bool MoveEdgeTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  // If dragging, do not highlight edges
  if (!(event->buttons() & Qt::LeftButton)) {
    // Reset edge highlighting for all edges
    for (EdgeItem* item : editor->itemsOfType<EdgeItem>()) {
      item->hoverLeave();
    }
    // Highlight all edges within the tool radius
    for (EdgeItem* item : editor->itemsInRing<EdgeItem>()) {
      item->hoverEnter();
    }
  }
  return false;
}

bool MoveEdgeTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  // Reset edge highlighting for all edges
  for (EdgeItem* item : editor->itemsOfType<EdgeItem>()) {
    item->hoverLeave();
  }
  // Highlight all edges within the tool radius
  for (EdgeItem* item : editor->itemsInRing<EdgeItem>()) {
    item->hoverEnter();
  }
  return false;
}
