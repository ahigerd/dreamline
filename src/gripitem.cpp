#include "gripitem.h"
#include "markeritem.h"
#include "meshitem.h"

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

void GripItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  MeshItem* mesh = dynamic_cast<MeshItem*>(parentItem());
  if (mesh && !mesh->verticesVisible()) {
    return;
  }
  MarkerItem::paint(painter, option, widget);
}
