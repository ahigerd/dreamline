#include "splitedge.h"
#include "editorview.h"
#include <QMouseEvent>

SplitEdgeTool::SplitEdgeTool()
: BaseTool()
{
}

Qt::CursorShape SplitEdgeTool::cursorShape() const
{
  return Qt::ArrowCursor;
}

bool SplitEdgeTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}

bool SplitEdgeTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}

bool SplitEdgeTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}
