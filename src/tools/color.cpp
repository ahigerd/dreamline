#include "color.h"
#include "editorview.h"
#include "gripitem.h"
#include <QMouseEvent>
#include <QColor>

ColorTool::ColorTool()
: BaseTool()
{
}

bool ColorTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  isDragging = true;
  QList<GripItem*> gripsInRing = editor->itemsInRing<GripItem>();
  for (GripItem* item : gripsInRing)
  {
    item->changeColor(editor->lastColor);
  }
  return false;
}

bool ColorTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  if (isDragging) {
    QList<GripItem*> gripsInRing = editor->itemsInRing<GripItem>();
    for (GripItem* item : gripsInRing)
    {
      item->changeColor(editor->lastColor);
    }
  }
  return false;
}

bool ColorTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  isDragging = false;
  return false;
}
