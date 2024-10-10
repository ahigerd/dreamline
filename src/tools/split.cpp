#include "split.h"
#include "editorview.h"
#include "meshitem.h"
#include "gripitem.h"
#include "edgeitem.h"
#include <QMouseEvent>
#include <QDebug>

SplitTool::SplitTool()
: BaseTool()
{
}

void SplitTool::activated(EditorView* editor)
{
  snapAndColorMarker(editor);
}

void SplitTool::deactivated(EditorView* editor)
{
  delete m_marker;
  m_marker = nullptr;
}

bool SplitTool::mousePressEvent(EditorView* editor, QMouseEvent* event)
{
  MeshItem* mesh = editor->activeMesh();
  GripItem* oldActive = editor->activeVertex();
  GripItem* snapGrip = editor->snapGrip();
  QPair<EdgeItem*, QPointF> snapEdge = editor->snapEdge();
  if (snapEdge.first || snapGrip) {
    if (!snapGrip) {
      snapEdge.first->split(snapEdge.second);
    }
    else {
      editor->setActiveVertex(snapGrip);
    }
    if (oldActive) {
      mesh->splitPolygon(oldActive, editor->activeVertex());
    }
  }

  return false;
}

bool SplitTool::mouseMoveEvent(EditorView* editor, QMouseEvent* event)
{
  snapAndColorMarker(editor);
  return false;
}

bool SplitTool::mouseReleaseEvent(EditorView* editor, QMouseEvent* event)
{
  return false;
}

void SplitTool::snapAndColorMarker(EditorView* editor)
{
  if (m_marker == nullptr) {
    m_marker = new MarkerItem();
    editor->scene()->addItem(m_marker);
    m_marker->setHighlight(QColor("#FF4444"));
  }

  GripItem* snapGrip = editor->snapGrip();
  QPair<EdgeItem*, QPointF> snapEdge = editor->snapEdge();

  if (snapGrip) {
    m_marker->setVisible(true);
    m_marker->setBrush(snapGrip->color());
    m_marker->setPos(snapGrip->pos());
  } else if (snapEdge.first) {
    m_marker->setVisible(true);
    m_marker->setBrush(snapEdge.first->colorAt(snapEdge.second));
    m_marker->setPos(snapEdge.second);
  } else {
    m_marker->setVisible(false);
  }
}
