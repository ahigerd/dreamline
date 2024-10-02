#ifndef DL_GRIPITEM_H
#define DL_GRIPITEM_H

#include <QGraphicsRectItem>
#include <QObject>
#include "markeritem.h"

class GripItem : public MarkerItem
{
Q_OBJECT
public:
  GripItem(QGraphicsItem* parent = nullptr);

signals:
  void moved(GripItem* item, const QPointF& pos);

protected:
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);
};

#endif
