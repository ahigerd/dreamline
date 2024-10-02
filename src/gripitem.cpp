#include "gripitem.h"
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
