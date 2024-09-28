#ifndef DL_TOOLS_MOVEEDGE_H
#define DL_TOOLS_MOVEEDGE_H

#include "tool.h"

class MoveEdgeTool : public BaseTool<MoveEdgeTool>
{
Q_OBJECT
public:
  MoveEdgeTool();

  virtual bool mousePressEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseMoveEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseReleaseEvent(EditorView* editor, QMouseEvent* event);
};

#endif
