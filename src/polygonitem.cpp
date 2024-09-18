#include "polygonitem.h"
#include "gripitem.h"

PolygonItem::PolygonItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsPolygonItem(parent)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);

  QPolygonF p;
  p.append(QPointF(-100, -100));
  p.append(QPointF(100, -50));
  p.append(QPointF(0, 100));
  setPolygon(p);

  for (int i = 0; i < p.length(); i++) {
    GripItem* grip = new GripItem(i, this);
    grip->setPos(p[i]);
    m_grips.append(grip);
    QObject::connect(grip, SIGNAL(moved(int, QPointF)), this, SLOT(moveVertex(int, QPointF)));
  }
}

void PolygonItem::moveVertex(int id, const QPointF& pos)
{
  QPolygonF p = polygon();
  p[id] = pos;
  setPolygon(p);
}
