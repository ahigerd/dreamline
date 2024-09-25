#include "gripitem.h"
#include <QContextMenuEvent>
#include <QGraphicsSceneHoverEvent>
#include <QColorDialog>
#include <QColor>
#include <QMenu>

GripItem::GripItem(int id, QGraphicsItem* parent)
: QObject(nullptr), QGraphicsRectItem(-3, -3, 6, 6, parent), m_id(id)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  setZValue(1);
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
