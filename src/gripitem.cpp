#include "gripitem.h"
#include <QContextMenuEvent>
#include <QGraphicsSceneHoverEvent>
#include <QColorDialog>
#include <QColor>
#include <QMenu>
#include <QPen>

GripItem::GripItem(int id, QGraphicsItem* parent)
: QObject(nullptr), QGraphicsRectItem(-3.5, -3.5, 7, 7, parent), m_id(id)
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

void GripItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
  QMenu menu;
  QAction* colorAction = menu.addAction(tr("&Color..."));
  QAction* selected = menu.exec(event->screenPos());
  if (selected == colorAction) {
    selectColor(event);
    return;
  }
}

void GripItem::selectColor(QGraphicsSceneContextMenuEvent* event)
{
  QColor color = QColorDialog::getColor(brush().color(), event->widget(), "Select Color");
  if (color.isValid())
  {
    setBrush(color);
    emit colorChanged(m_id, color);
  }
}
