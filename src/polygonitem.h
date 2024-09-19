#ifndef DL_POLYGONITEM_H
#define DL_POLYGONITEM_H

#include <QGraphicsPolygonItem>
#include <QOpenGLBuffer>
#include <QObject>
#include <QVector>
#include "glbuffer.h"
class GripItem;

class PolygonItem : public QObject, public QGraphicsPolygonItem
{
Q_OBJECT
public:
  PolygonItem(QGraphicsItem* parent = nullptr);

public slots:
  void moveVertex(int id, const QPointF& pos);

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
  void updateVertices();
  void updateColors();

  QVector<GripItem*> m_grips;
  bool m_vboDirty, m_colorsDirty;
  GLBuffer<QPointF> m_vbo;
  GLBuffer<QColor> m_colorBuffer;
};

#endif
