#ifndef DL_POLYLINEITEM_H
#define DL_POLYLINEITEM_H

#include <QGraphicsPolygonItem>
#include <QObject>
class QGraphicsLineItem;
class GripItem;
class EdgeItem;

class PolyLineItem : public QObject, public QGraphicsPolygonItem
{
Q_OBJECT
public:
  PolyLineItem(QGraphicsItem* parent = nullptr);

  int pointCount() const;

  QPointF point(int index) const;
  void setPoint(int index, const QPointF& pos);
  GripItem* addPoint(const QPointF& pos);
  void addPoint(GripItem* vertex);

  GripItem* grip(int index) const;

  void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* = nullptr);

private slots:
  void updatePolygon();

private:

  QList<GripItem*> vertices;
};

#endif
