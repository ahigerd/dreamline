#include "moveedge.h"
#include "editorview.h"
#include "edgeitem.h"
#include <QMouseEvent>

MoveEdgeTool::MoveEdgeTool()
: BaseTool()
{
}

bool MoveEdgeTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  for (EdgeItem* item : editor->selectedItems<EdgeItem>()) {
    item->setSelected(false);
  }
  QList<EdgeItem*> gripsInRing = editor->itemsInRing<EdgeItem>();
  for (EdgeItem* item : gripsInRing)
  {
    item->setSelected(true);
  }
  return true;
}

bool MoveEdgeTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  return true;
}

bool MoveEdgeTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return true;
}
