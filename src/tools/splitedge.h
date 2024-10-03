#ifndef DL_TOOLS_SPLITEDGE_H
#define DL_TOOLS_SPLITEDGE_H

#include "tool.h"

class SplitEdgeTool : public BaseTool<SplitEdgeTool>
{
Q_OBJECT
public:
  SplitEdgeTool();

  virtual bool mousePressEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseMoveEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseReleaseEvent(EditorView* editor, QMouseEvent* event);
};

#endif
