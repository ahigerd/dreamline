#include "movevertex.h"
#include "editorview.h"
#include "gripitem.h"
#include <QMouseEvent>

MoveVertexTool::MoveVertexTool()
: BaseTool()
{
}

bool MoveVertexTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  // Only clear selection on left click if Shift or Ctrl are not clicked
  if (!(event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier)) {
    for (GripItem* grip : editor->selectedItems<GripItem>()) {
      grip->setSelected(false);
    }
  }
  QList<GripItem*> gripsInRing = editor->itemsInRing<GripItem>();
  if (event->modifiers() & Qt::ControlModifier) {
    // If Ctrl is held unselect grips in ring
    for (GripItem* item : gripsInRing) {
      item->setSelected(false);
    }
  } else {
    // Otherwise select grips in ring
    for (GripItem* item : gripsInRing) {
      item->setSelected(true);
    }
  }
  if (gripsInRing.length() == 1) {
    // Only change the active vertex if there is only a single grip in the ring.
    // Will probably remove this functionality later since active vertex is probably
    // only relevant to the mesh and split tool which can set this through snapping.
    editor->setActiveVertex(gripsInRing[0]);
  } else {
    // If there are multiple vertices in the ring, clear the active vertex.
    editor->setActiveVertex(nullptr);
  }
  return false;
}

bool MoveVertexTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}

bool MoveVertexTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}
