#ifndef DL_GRIPITEM_H
#define DL_GRIPITEM_H

#include <QGraphicsRectItem>
#include <QObject>

class GripItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT
public:
  GripItem(int id, QGraphicsItem* parent = nullptr);

  void reindex(int newId);

signals:
  void moved(int id, const QPointF& pos);

protected:
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);

private:
  int m_id;
};

#endif
