#ifndef DL_MESHITEM_H
#define DL_MESHITEM_H

#include <QGraphicsPolygonItem>
#include <QObject>
#include <QColor>
#include <QJsonObject>
#include <QPen>
#include <memory>
#include "markeritem.h"
class PropertyPanel;
class GripItem;
class EdgeItem;
class PolyLineItem;
class MeshItemPrivate;
class DreamProject;
class EditorView;

class MeshItem : public QObject, public QGraphicsPolygonItem
{
Q_OBJECT
public:
  MeshItem(QGraphicsItem* parent = nullptr);
  MeshItem(PolyLineItem* polyline, QGraphicsItem* parent = nullptr);
  MeshItem(const QJsonObject& source, QGraphicsItem* parent = nullptr);
  ~MeshItem();

  QJsonObject serialize() const;

  bool edgesVisible() const;
  void setEdgesVisible(bool on);
  bool verticesVisible() const;
  void setVerticesVisible(bool on);

  int numBoundaryVertices() const;
  GripItem* boundaryVertex(int index) const;
  GripItem* activeVertex() const;
  bool splitPolygon(GripItem* v1, GripItem* v2);
  bool splitPolygon(GripItem* vertex, EdgeItem* edge);

  QPen strokePen() const;
  PropertyPanel* fillPropertyPanel();
  PropertyPanel* strokePropertyPanel();

public slots:
  void moveVertex(GripItem* vertex, const QPointF& pos);
  void changeColor(MarkerItem* vertex, const QColor& color);
  void insertVertex(EdgeItem* edge, const QPointF& pos);
  void setActiveVertex(GripItem* vertex);
  void addPolygon(PolyLineItem* poly);
  void setStrokePen(const QPen& pen);
  void removeStroke();

protected slots:
  void gripDestroyed(QObject* grip);
  void updateBoundary();

signals:
  void modified(bool);

protected:
  DreamProject* project() const;
  EditorView* editor() const;
  GripItem* newGrip();
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
  friend class MeshItemPrivate;
  MeshItemPrivate* d;
};

#endif
