#ifndef DL_MESHITEM_H
#define DL_MESHITEM_H

#include <QGraphicsPolygonItem>
#include <QOpenGLBuffer>
#include <QObject>
#include <QVector>
#include <QColor>
#include "glbuffer.h"
class GripItem;
class EdgeItem;

class MeshItem : public QObject, public QGraphicsPolygonItem
{
Q_OBJECT
public:
  MeshItem(QGraphicsItem* parent = nullptr);

public slots:
  void moveVertex(GripItem* vertex, const QPointF& pos);
  void changeColor(GripItem* vertex, const QColor& color);
  void insertVertex(EdgeItem* edge, const QPointF& pos);

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
  QVector<GripItem*> m_grips;
  QVector<EdgeItem*> m_edges;
  GLBuffer<QPointF> m_vbo;
  GLBuffer<QColor> m_colorBuffer;
};

#endif
