#include "gripitem.h"
#include <QContextMenuEvent>
#include <QGraphicsSceneHoverEvent>
#include <QStyleOptionGraphicsItem>
#include <QColorDialog>
#include <QColor>
#include <QMenu>
#include <QPen>
#include <QPainter>
#include "markeritem.h"

GripItem::GripItem(QGraphicsItem* parent) : MarkerItem(parent)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QVariant GripItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  if (change == ItemPositionHasChanged) {
    emit moved(this, value.toPointF());
  }
  return value;
}
