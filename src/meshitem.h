#ifndef DL_MESHITEM_H
#define DL_MESHITEM_H

#include <QGraphicsPolygonItem>
#include <QOpenGLBuffer>
#include <QObject>
#include <QVector>
#include <QColor>
#include <QVector4D>
#include <QPointer>
#include <QSet>
#include "glbuffer.h"
class GripItem;
class EdgeItem;

class MeshItem : public QObject, public QGraphicsPolygonItem
{
Q_OBJECT
public:
  MeshItem(QGraphicsItem* parent = nullptr);

  GripItem* activeVertex() const;
  GripItem* addVertexToPolygon(const QPointF& pos);
  bool splitPolygon(GripItem* v1, GripItem* v2);
  bool splitPolygon(GripItem* vertex, EdgeItem* edge);

public slots:
  void moveVertex(GripItem* vertex, const QPointF& pos);
  void changeColor(GripItem* vertex, const QColor& color);
  void insertVertex(EdgeItem* edge, const QPointF& pos);
  void setActiveVertex(GripItem* vertex);

protected slots:
  void gripDestroyed(QObject* grip);

protected:
  GripItem* newGrip();
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
  struct Polygon {
  public:
    Polygon();
    QVector<GripItem*> vertices;
    QVector<EdgeItem*> edges;
    GLBuffer<QPointF> vertexBuffer;
    QVector<QVector4D> colors;
    GLfloat windingDirection;

    bool insertVertex(GripItem* vertex, EdgeItem* oldEdge, EdgeItem* newEdge);

    QColor color(int index) const;
    void setColor(int index, const QColor& color);

    void updateWindingDirection();
    void rebuildBuffers();

    QSet<EdgeItem*> edgesContainingVertex(GripItem* vertex) const;
    bool isEdgeInside(GripItem* v1, GripItem* v2) const;

  private:
    bool testEdge(GripItem* v1, GripItem* v2, EdgeItem* edge1, EdgeItem* edge2) const;
  };

  QSet<Polygon*> polygonsContainingVertex(GripItem* vertex);
  Polygon* findSplittablePolygon(GripItem* v1, GripItem* v2);

  QVector<GripItem*> m_grips, m_boundary;
  QVector<EdgeItem*> m_edges;
  QList<Polygon> m_polygons;
  QPointer<GripItem> m_lastVertex;
  QGraphicsEllipseItem* m_lastVertexFocus;
};

#endif
