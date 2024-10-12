#ifndef DL_TOOLS_COLOR_H
#define DL_TOOLS_COLOR_H

#include "tool.h"

class ColorTool : public BaseTool<ColorTool>
{
Q_OBJECT
public:
  ColorTool();

  virtual bool mousePressEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseMoveEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseReleaseEvent(EditorView* editor, QMouseEvent* event);
};

#endif
