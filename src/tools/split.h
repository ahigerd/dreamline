#ifndef DL_TOOLS_SPLITEDGE_H
#define DL_TOOLS_SPLITEDGE_H

#include "tool.h"
#include "markeritem.h"

class SplitTool : public BaseTool<SplitTool>
{
Q_OBJECT
public:
  SplitTool();

  virtual void activated(EditorView* editor);
  virtual void deactivated(EditorView* editor);

  virtual bool mousePressEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseMoveEvent(EditorView* editor, QMouseEvent* event);
  virtual bool mouseReleaseEvent(EditorView* editor, QMouseEvent* event);

private:
  MarkerItem* m_marker = nullptr;

  void snapAndColorMarker(EditorView* editor);
};

#endif
