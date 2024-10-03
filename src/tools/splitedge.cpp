#include "splitedge.h"
#include "editorview.h"
#include "meshitem.h"
#include "gripitem.h"
#include <QMouseEvent>

SplitEdgeTool::SplitEdgeTool()
: BaseTool()
{
}

bool SplitEdgeTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  GripItem* grip = dynamic_cast<GripItem*>(editor->itemAt(event->pos()));
  MeshItem* mesh = editor->activeMesh();
  if (grip) {
    if (mesh && (event->modifiers() & Qt::ShiftModifier) && mesh->activeVertex() != grip) {
      mesh->splitPolygon(mesh->activeVertex(), grip);
      return true;
    } else {
      editor->setActiveVertex(grip);
    }
  } else {
    editor->setActiveVertex(nullptr);
  }
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
