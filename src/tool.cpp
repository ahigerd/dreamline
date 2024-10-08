#include "tool.h"
#include "tools/movevertex.h"
#include "tools/moveedge.h"
#include "tools/color.h"
#include "tools/split.h"
#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QStyle>
#include <QKeySequence>

using ToolConstructor = Tool*(*)();

// XXX: This is a lot more complicated than it needs to be. When I remove
// the QStyle::StandardPixmap stuff this will mostly disappear.
struct ToolData {
  QString name;
  QString iconPath;
  QKeySequence shortcut;
  ToolConstructor ctor;
};

static QMap<Tool::Type, ToolData> toolData;
static QMap<Tool::Type, Tool*> tools;

// A static member function is used in order to establish a tr() context
void Tool::initializeToolData()
{
  if (!toolData.isEmpty()) {
    return;
  }

  toolData = QMap<Tool::Type, ToolData>{
    { VertexTool, { tr("&Vertex Tool"), "", QKeySequence("v"), &MoveVertexTool::create } },
    { EdgeTool,   { tr("&Edge Tool"),   "", QKeySequence("e"), &MoveEdgeTool::create } },
    { ColorTool,  { tr("&Color Tool"),  "", QKeySequence("c"), &ColorTool::create } },
    { Split,      { tr("&Split Tool"),  "", QKeySequence("s"), &SplitTool::create } },
  };
}

QAction* Tool::makeAction(QActionGroup* group, Tool::Type type)
{
  initializeToolData();

  ToolData data = toolData[type];

  QIcon icon(data.iconPath);
  QAction* action = new QAction(icon, data.name, group);
  action->setCheckable(true);
  action->setShortcut(data.shortcut);
  action->setData(type);
  group->addAction(action);

  return action;
}

Tool* Tool::get(Tool::Type type)
{
  Tool* tool = tools[type];
  if (!tool) {
    initializeToolData();

    tools[type] = tool = toolData[type].ctor();
  }
  return tool;
}

Tool::Tool()
: QObject(nullptr)
{
  // initializers only
}

Qt::CursorShape Tool::cursorShape() const
{
  // This special value means to use the selection ring
  return Qt::BitmapCursor;
}

void Tool::activated(EditorView*)
{
  // no-op
}

void Tool::deactivated(EditorView*)
{
  // no-op
}
