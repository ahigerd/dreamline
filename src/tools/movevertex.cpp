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
  for (GripItem* grip : editor->selectedItems<GripItem>()) {
    grip->setSelected(false);
  }
  QList<GripItem*> gripsInRing = editor->itemsInRing<GripItem>();
  for (GripItem* item : gripsInRing) {
    item->setSelected(true);
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
