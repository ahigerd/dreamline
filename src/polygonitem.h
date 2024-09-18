#ifndef DL_POLYGONITEM_H
#define DL_POLYGONITEM_H

#include <QGraphicsPolygonItem>
#include <QObject>
#include <QVector>
class GripItem;

class PolygonItem : public QObject, public QGraphicsPolygonItem
{
Q_OBJECT
public:
  PolygonItem(QGraphicsItem* parent = nullptr);

public slots:
  void moveVertex(int id, const QPointF& pos);

private:
  QVector<GripItem*> m_grips;
};

#endif
