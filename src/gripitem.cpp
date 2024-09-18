#include "gripitem.h"

GripItem::GripItem(int id, QGraphicsItem* parent)
: QObject(nullptr), QGraphicsRectItem(-3, -3, 6, 6, parent), m_id(id)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

QVariant GripItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  if (change == ItemPositionHasChanged) {
    emit moved(m_id, value.toPointF());
  }
  return value;
}
