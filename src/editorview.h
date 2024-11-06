#ifndef DL_EDITORVIEW_H
#define DL_EDITORVIEW_H

#include <QMouseEvent>
#include <QGraphicsView>
#include <QElapsedTimer>
#include <QPainterPath>
#include "tool.h"
#include "dreamproject.h"
class QPinchGesture;
class GLViewport;
class GripItem;
class EdgeItem;
class MeshItem;
class PropertyPanel;

class EditorView : public QGraphicsView
{
Q_OBJECT
public:
  EditorView(QWidget* parent = nullptr);

  void newProject();

  QPointF cursorPos() const;

  QList<QGraphicsItem*> itemsInRing() const;

  template <typename ItemType>
  inline QList<ItemType*> itemsOfType() const
  {
    return projectScene->itemsOfType<ItemType>();
  }

  template <typename ItemType>
  inline QList<ItemType*> selectedItems() const
  {
    return projectScene->selectedItems<ItemType>();
  }

  template <typename ItemType>
  QList<ItemType*> itemsInRing() const
  {
    return DreamProject::filterItemsByType<ItemType>(itemsInRing());
  }

  GripItem* activeVertex() const;
  MeshItem* activeMesh() const;
  GripItem* snapGrip() const;
  QPair<EdgeItem*, QPointF> snapEdge() const;

  bool edgesVisible() const;
  void setEdgesVisible(bool on);
  bool verticesVisible() const;
  void setVerticesVisible(bool on);

  bool isPreview() const;

  DreamProject* project() const;

  QColor color() const;
  QAction* colorAction() const;

public slots:
  void setPreview(bool on);
  void setTool(QAction* toolAction);
  void setTool(Tool::Type type);
  void setActiveVertex(GripItem* vertex);
  void setColor(const QColor& color);
  void selectColor();
  void toggleSmooth();

signals:
  void projectModified(bool);
  void colorSelected(const QColor&);
  void selectionChanged();
  void propertyPanelChanged(const QString& tag, PropertyPanel* panel);

protected:
  bool viewportEvent(QEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void wheelEvent(QWheelEvent* event);
  void drawForeground(QPainter* p, const QRectF& rect);

private:
  void pinchGesture(QPinchGesture* gesture);
  void updateMouseRect();
  void setCursorFromTool();

  void contextMenu(const QPoint& pos);

  QElapsedTimer timer;
  GLViewport* glViewport;
  DreamProject* projectScene;
  bool isPanning, isResizingRing, containsMouse, useRing;
  bool m_edgesVisible = true;
  bool m_verticesVisible = true;
  bool m_preview = false;
  float ringSize, originalRingSize;
  QPoint dragStart, lastDrag;
  QRectF lastMouseRect;
  Tool* currentTool;
  QGraphicsRectItem* underCursor;
  QColor lastColor;
  QAction* m_colorAction;
};

#endif
