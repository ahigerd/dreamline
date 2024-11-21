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
  // Clear entire selection if Ctrl is not held
  if (!(event->modifiers() & Qt::ControlModifier)) {
    for (GripItem* grip : editor->selectedItems<GripItem>()) {
      grip->setSelected(false);
    }
  }
  // Select all grips within the tool radius
  QList<GripItem*> gripsInRing = editor->itemsInRing<GripItem>();
  bool allSelected = true;
  for (GripItem* item : gripsInRing) {
    allSelected = allSelected && item->isSelected();
    item->setSelected(true);
  }
  // If ctrl is held and all grips within the ring were selected prior to clicking
  // queue them to be unselected on mouse release
  if (allSelected && (event->modifiers() & Qt::ControlModifier)) {
    for (GripItem* item : gripsInRing) {
      gripsToDeselect.append(item);
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
  // If dragging occurse, cancel deselection of grips
  if (event->buttons() & Qt::LeftButton) {
    gripsToDeselect.clear();
  }
  return false;
}

bool MoveVertexTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  // Deselect grips queued to get deselected
  for (GripItem* item : gripsToDeselect) {
    if (item != nullptr) {
      item->setSelected(false);
    }
  }
  // Clear queue
  gripsToDeselect.clear();
  return false;
}
