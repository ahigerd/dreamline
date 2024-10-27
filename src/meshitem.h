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
#include "glbuffer.h"
#include "markeritem.h"
class GripItem;
class EdgeItem;
class PolyLineItem;

class MeshItem : public QObject, public QGraphicsPolygonItem
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

  void renderGL();

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
  struct Polygon {
  public:
    Polygon();
    QVector<GripItem*> vertices;
    QVector<EdgeItem*> edges;
    GLBuffer<QVector2D> vertexBuffer;
    GLBuffer<QVector4D> colors;
    GLfloat windingDirection;

    bool insertVertex(GripItem* vertex, EdgeItem* oldEdge, EdgeItem* newEdge);

    inline QPointF vertex(int index) const { return vertexBuffer[index].toPointF(); }
    inline void setVertex(int index, const QPointF& pos) { vertexBuffer[index] = QVector2D(pos); }

    QColor color(int index) const;
    void setColor(int index, const QColor& color);

    void updateWindingDirection();
    void rebuildBuffers();

    QSet<EdgeItem*> edgesContainingVertex(GripItem* vertex) const;
    bool isEdgeInside(GripItem* v1, GripItem* v2) const;

    QString debug() const;

  private:
    bool testEdge(GripItem* v1, GripItem* v2, EdgeItem* edge1, EdgeItem* edge2) const;
  };

  QSet<Polygon*> polygonsContainingVertex(GripItem* vertex);
  Polygon* findSplittablePolygon(GripItem* v1, GripItem* v2);
  EdgeItem* findOrCreateEdge(GripItem* v1, GripItem* v2);
  void recomputeBoundaries();

  QVector<GripItem*> m_grips, m_boundary;
  QVector<EdgeItem*> m_edges;
  QList<Polygon> m_polygons;
  GLBuffer<QPointF> m_boundaryTris, m_control;
  GLBuffer<GLint> m_smooth;
  QPointer<GripItem> m_lastVertex;
  QGraphicsEllipseItem* m_lastVertexFocus;
  bool m_edgesVisible, m_verticesVisible;
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
