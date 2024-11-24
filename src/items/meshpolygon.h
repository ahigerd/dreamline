#ifndef DL_MESHPOLYGON_H
#define DL_MESHPOLYGON_H

#include <QVector2D>
#include <QVector4D>
#include <QPointF>
#include <QColor>
#include <QSet>
#include "glbuffer.h"
class GripItem;
class EdgeItem;
class MeshItem;

class MeshPolygon {
public:
  MeshPolygon(MeshItem* owner);
  QVector<GripItem*> vertices;
  QVector<EdgeItem*> edges;
  GLBuffer<QVector2D> vertexBuffer, colorPoints;
  GLBuffer<QVector4D> colors;
  GLfloat windingDirection;

  bool insertVertex(GripItem* vertex, EdgeItem* oldEdge, EdgeItem* newEdge);

  inline QPointF vertex(int index) const { return vertexBuffer[index].toPointF(); }
  void setVertex(int index, const QPointF& pos);

  QColor color(int index) const;
  void setColor(int index, const QColor& color);

  void updateWindingDirection();
  void rebuildBuffers();

  QSet<EdgeItem*> edgesContainingVertex(GripItem* vertex) const;
  bool isEdgeInside(GripItem* v1, GripItem* v2) const;

  QString debug() const;

private:
  bool testEdge(GripItem* v1, GripItem* v2, EdgeItem* edge1, EdgeItem* edge2) const;
  MeshItem* owner;
};

#endif
