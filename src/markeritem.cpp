#include "markeritem.h"
#include <QContextMenuEvent>
#include <QGraphicsSceneHoverEvent>
#include <QStyleOptionGraphicsItem>
#include <QColorDialog>
#include <QColor>
#include <QMenu>
#include <QPen>
#include <QPainter>

MarkerItem::MarkerItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsRectItem(-4.5, -4.5, 9, 9, parent)
{
  setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  setZValue(1);
  QPen pen(Qt::black, 1);
  pen.setCosmetic(true);
  setPen(pen);
}


QColor MarkerItem::color() const
{
  return brush().color();
}

void MarkerItem::setColor(const QColor& color)
{
  setBrush(color);
  emit colorChanged(this, color);
}

void MarkerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
  // Snap coordinates to device pixels to avoid fuzzy edges
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
