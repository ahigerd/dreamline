#include "gripitem.h"
#include <QContextMenuEvent>
#include <QGraphicsSceneHoverEvent>
#include <QStyleOptionGraphicsItem>
#include <QColorDialog>
#include <QColor>
#include <QMenu>
#include <QPen>
#include <QPainter>

GripItem::GripItem(int id, QGraphicsItem* parent)
: QObject(nullptr), QGraphicsRectItem(-4.5, -4.5, 9, 9, parent), m_id(id)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  setZValue(1);
  QPen pen(Qt::black, 1);
  pen.setCosmetic(true);
  setPen(pen);
}

QVariant GripItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  if (change == ItemPositionHasChanged) {
    emit moved(m_id, value.toPointF());
  }
  return value;
}

void GripItem::reindex(int newId)
{
  m_id = newId;
}


void GripItem::changeColor(const QColor& color)
{
  setBrush(color);
  emit colorChanged(m_id, color);
}

void GripItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
  double xFrac = painter->deviceTransform().dx();
  xFrac = xFrac - int(xFrac);

  double yFrac = painter->deviceTransform().dy();
  yFrac = yFrac - int(yFrac);

  QRectF r = rect().adjusted(-xFrac, -yFrac, -xFrac, -yFrac);
  if (option->state & QStyle::State_Selected) {
    painter->setPen(option->palette.color(QPalette::Highlight));
    painter->drawRect(r);
  }
  painter->setPen(pen());
  painter->setBrush(brush());
  painter->drawRect(r.adjusted(1, 1, -1, -1));
}
