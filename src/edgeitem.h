#ifndef DL_EDGEITEM_H
#define DL_EDGEITEM_H

#include <QGraphicsLineItem>
#include <QObject>
class GripItem;
class QGraphicsPathItem;

class EdgeItem : public QObject, public QGraphicsLineItem
{
Q_OBJECT
public:
  EdgeItem(GripItem* left, GripItem* right);

  QPainterPath shape() const;
  void updateShape();

signals:
  void insertVertex(EdgeItem*, const QPointF&);

protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent*);
  void hoverMoveEvent(QGraphicsSceneHoverEvent*);
  void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
  void mousePressEvent(QGraphicsSceneMouseEvent*);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);
};

#endif
