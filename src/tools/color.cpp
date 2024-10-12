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
  // Set color of all grips within the tool radius
  for (GripItem* item : editor->itemsInRing<GripItem>())
  {
    item->setColor(editor->color());
  }
  return true;
}

bool ColorTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  // Allow drag to color
  if (event->buttons() & Qt::LeftButton) {
    for (GripItem* item : editor->itemsInRing<GripItem>())
    {
      item->setColor(editor->color());
    }
  }
  return true;
}

bool ColorTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return true;
}
