#ifndef DL_GRIPITEM_H
#define DL_GRIPITEM_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QColor>

class GripItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT
public:
  GripItem(QGraphicsItem* parent = nullptr);

  QColor color() const;

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*);

public slots:
  void setColor(const QColor& color);

signals:
  void moved(GripItem* item, const QPointF& pos);
  void colorChanged(GripItem* item, const QColor& pos);

protected:
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);
};

#endif
