#ifndef DL_MESHITEM_H
#define DL_MESHITEM_H

#include <QGraphicsPolygonItem>
#include <QOpenGLBuffer>
#include <QObject>
#include <QVector>
#include <QColor>
#include <QVector4D>
#include <QPointer>
#include "glbuffer.h"
class GripItem;
class EdgeItem;

class MeshItem : public QObject, public QGraphicsPolygonItem
{
Q_OBJECT
public:
  MeshItem(QGraphicsItem* parent = nullptr);

  GripItem* activeVertex() const;

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
    QVector<GripItem*> vertices;
    QVector<EdgeItem*> edges;
    GLBuffer<QPointF> vertexBuffer;
    QVector<QVector4D> colors;

    bool insertVertex(GripItem* vertex, EdgeItem* oldEdge, EdgeItem* newEdge);
    QColor color(int index) const;
    void setColor(int index, const QColor& color);
  };

  QVector<GripItem*> m_grips, m_boundary;
  QVector<EdgeItem*> m_edges;
  QList<Polygon> m_polygons;
  QPointer<GripItem> m_lastVertex;
  QGraphicsEllipseItem* m_lastVertexFocus;
};

#endif
