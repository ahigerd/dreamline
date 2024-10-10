#ifndef DL_EDITORVIEW_H
#define DL_EDITORVIEW_H

#include <QMouseEvent>
#include <QGraphicsView>
#include <QElapsedTimer>
#include <QPainterPath>
#include <stdexcept>
#include "tool.h"
class DreamProject;
class QPinchGesture;
class GLViewport;
class GripItem;
class EdgeItem;
class MeshItem;

class OpenException : public std::runtime_error
{
public:
  OpenException(const QString& what);
};

class SaveException : public std::runtime_error
{
public:
  SaveException(const QString& what);
};

class EditorView : public QGraphicsView
{
Q_OBJECT
public:
  EditorView(QWidget* parent = nullptr);

  void newProject();
  void openProject(const QString& path);
  void saveProject(const QString& path);

  QPointF cursorPos() const;

  QColor lastColor = Qt::blue;

  QList<QGraphicsItem*> itemsInRing() const;

  template <typename ItemType>
  static QList<ItemType*> filterItemsByType(const QList<QGraphicsItem*>& items)
  {
    QList<ItemType*> result;
    for (QGraphicsItem* genericItem : items) {
      ItemType* item = dynamic_cast<ItemType*>(genericItem);
      if (item) {
        result << item;
      }
    }
    return result;
  }

  template <typename ItemType>
  QList<ItemType*> itemsOfType() const
  {
    return filterItemsByType<ItemType>(scene()->items());
  }

  template <typename ItemType>
  QList<ItemType*> selectedItems() const
  {
    return filterItemsByType<ItemType>(scene()->selectedItems());
  }

  template <typename ItemType>
  QList<ItemType*> itemsInRing() const
  {
    return filterItemsByType<ItemType>(itemsInRing());
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

public slots:
  void setPreview(bool on);
  void setTool(QAction* toolAction);
  void setTool(Tool::Type type);
  void setActiveVertex(GripItem* vertex);
  void selectColor();
  void toggleSmooth();

signals:
  void projectModified(bool);
  void colorSelected(const QColor&);

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
};

#endif
