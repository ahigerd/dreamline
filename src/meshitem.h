#ifndef DL_MESHITEM_H
#define DL_MESHITEM_H

#include <QGraphicsPolygonItem>
#include <QObject>
#include <QVector>
#include <QColor>
#include <QVector2D>
#include <QVector4D>
#include <QPointer>
#include <QSet>
#include <QJsonObject>
#include <memory>
#include "glbuffer.h"
#include "markeritem.h"
#include "meshpolygon.h"
#include "abstractmeshrenderer.h"
class GripItem;
class EdgeItem;
class PolyLineItem;

class MeshRenderData
{
public:
  QList<MeshPolygon> polygons;
  GLBuffer<QPointF> boundaryTris, controlPoints;
};

class MeshItem : public QObject, public QGraphicsPolygonItem, private MeshRenderData
{
Q_OBJECT
public:
  MeshItem(QGraphicsItem* parent = nullptr);
  MeshItem(PolyLineItem* polyline, QGraphicsItem* parent = nullptr);
  MeshItem(const QJsonObject& source, QGraphicsItem* parent = nullptr);

  QJsonObject serialize() const;

  bool edgesVisible() const;
  void setEdgesVisible(bool on);
  bool verticesVisible() const;
  void setVerticesVisible(bool on);

  GripItem* activeVertex() const;
  bool splitPolygon(GripItem* v1, GripItem* v2);
  bool splitPolygon(GripItem* vertex, EdgeItem* edge);

public slots:
  void moveVertex(GripItem* vertex, const QPointF& pos);
  void changeColor(MarkerItem* vertex, const QColor& color);
  void insertVertex(EdgeItem* edge, const QPointF& pos);
  void setActiveVertex(GripItem* vertex);
  void addPolygon(PolyLineItem* poly);

protected slots:
  void gripDestroyed(QObject* grip);
  void updateBoundary();

signals:
  void modified(bool);

protected:
  GripItem* newGrip();
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
  QSet<MeshPolygon*> polygonsContainingVertex(GripItem* vertex);
  MeshPolygon* findSplittablePolygon(GripItem* v1, GripItem* v2);
  EdgeItem* findOrCreateEdge(GripItem* v1, GripItem* v2);
  void recomputeBoundaries();

  QVector<GripItem*> m_grips, m_boundary;
  QVector<EdgeItem*> m_edges;
  GLBuffer<GLint> m_smooth;
  QPointer<GripItem> m_lastVertex;
  QGraphicsEllipseItem* m_lastVertexFocus;
  bool m_edgesVisible, m_verticesVisible;

  std::unique_ptr<AbstractMeshRenderer> m_fill;
  std::unique_ptr<AbstractMeshRenderer> m_stroke;
};

inline bool operator==(const QVector2D& lhs, const QPointF& rhs)
{
  return lhs == QVector2D(rhs);
}

inline bool operator==(const QPointF& lhs, const QVector2D& rhs)
{
  return rhs == QVector2D(lhs);
}

#endif
