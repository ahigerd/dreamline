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
  if (!(event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier)) {
    for (GripItem* grip : editor->selectedItems<GripItem>()) {
      grip->setSelected(false);
    }
  }
  QList<GripItem*> gripsInRing = editor->itemsInRing<GripItem>();
  if (event->modifiers() & Qt::ControlModifier) {
    for (GripItem* item : gripsInRing) {
      item->setSelected(false);
    }
  } else {
    for (GripItem* item : gripsInRing) {
      item->setSelected(true);
    }
  }
  if (gripsInRing.length() == 1) {
    editor->setActiveVertex(gripsInRing[0]);
  } else {
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
