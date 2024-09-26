#ifndef DL_GRIPITEM_H
#define DL_GRIPITEM_H

#include "qcolor.h"
#include <QGraphicsRectItem>
#include <QObject>
#include <QColor>

class GripItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT
public:
  GripItem(int id, QGraphicsItem* parent = nullptr);

  void reindex(int newId);

  void changeColor(QColor& color);


signals:
  void moved(int id, const QPointF& pos);
  void colorChanged(int id, const QColor& pos);

protected:
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);

private:
  int m_id;

};

#endif
