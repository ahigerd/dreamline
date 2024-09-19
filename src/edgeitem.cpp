#include "edgeitem.h"
#include "gripitem.h"
#include <QGraphicsSceneHoverEvent>
#include <QPen>
#include <QtDebug>

EdgeItem::EdgeItem(GripItem* left, GripItem* right)
: QObject(nullptr), QGraphicsLineItem(QLineF(left->pos(), right->pos()), left->parentItem())
{
  setAcceptHoverEvents(true);
  hoverLeaveEvent(nullptr);
  setZValue(-1);
}

void EdgeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  QPointF mouse = event->pos();
  QPointF left = mapFromItem(parentItem(), line().p1());
  QPointF right = mapFromItem(parentItem(), line().p2());
  if (QLineF(mouse, left).length() <= 4 || QLineF(mouse, right).length() <= 4) {
    hoverLeaveEvent(event);
    return;
  }
  m_hovered = true;
  QPen pen(Qt::black, 5);
  pen.setCosmetic(true);
  setPen(pen);
}

void EdgeItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
  hoverEnterEvent(event);
}

void EdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
  m_hovered = false;
  QPen pen(Qt::transparent, 5);
  pen.setCosmetic(true);
  setPen(pen);
}

void EdgeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
  if (!m_hovered) {
    return;
  }
  emit insertVertex(this, event->scenePos());
}
