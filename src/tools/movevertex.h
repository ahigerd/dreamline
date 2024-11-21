#ifndef DL_TOOLS_MOVEVERTEX_H
#define DL_TOOLS_MOVEVERTEX_H

#include "tool.h"
#include "gripitem.h"
#include <QPointer>

class MoveVertexTool : public BaseTool<MoveVertexTool>
{
Q_OBJECT
public:
  MoveVertexTool();

  virtual bool mousePressEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseMoveEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseReleaseEvent(EditorView* editor, QMouseEvent* event);

private:
  QList<QPointer<GripItem>> gripsToDeselect;
};

#endif
