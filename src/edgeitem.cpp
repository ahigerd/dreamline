#include "edgeitem.h"
#include "gripitem.h"
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsPathItem>
#include <QPen>

EdgeItem::EdgeItem(GripItem* left, GripItem* right)
: QObject(nullptr), QGraphicsLineItem(QLineF(left->pos(), right->pos()), left->parentItem())
{
  setAcceptHoverEvents(true);
  hoverLeaveEvent(nullptr);
  setZValue(0.1);
}

QPainterPath EdgeItem::shape() const
{
  QPainterPath path;

  QLineF normVector = line().normalVector();
  normVector.setLength(2);
  QPointF norm = normVector.p2() - normVector.p1();

  QLineF capVector = line();
  capVector.setLength(3.5);
  QPointF cap = capVector.p2() - capVector.p1();

  QPointF p1 = mapFromItem(parentItem(), line().p1() + cap);
  QPointF p2 = mapFromItem(parentItem(), line().p2() - cap);
  path.moveTo(p1 - norm);
  path.lineTo(p1 + norm);
  path.lineTo(p2 + norm);
  path.lineTo(p2 - norm);
  path.closeSubpath();
  return path;
}

void EdgeItem::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
  QPen pen(Qt::black, 3);
  pen.setCosmetic(true);
  setPen(pen);
}

void EdgeItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
  hoverEnterEvent(event);
}

void EdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
  QPen pen(Qt::transparent, 5);
  pen.setCosmetic(true);
  setPen(pen);
}

void EdgeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  event->accept();
}

void EdgeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
  emit insertVertex(this, event->scenePos());
  event->accept();
}
