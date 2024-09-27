#include "color.h"
#include "editorview.h"
#include <QMouseEvent>

ColorTool::ColorTool()
: BaseTool()
{
}

bool ColorTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}

bool ColorTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}

bool ColorTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}
