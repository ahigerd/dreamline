#include "polylineitem.h"
#include "gripitem.h"
#include <QPen>
#include <QPainter>

PolyLineItem::PolyLineItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsPolygonItem(parent)
{
  QPen pen(Qt::black, 0);
  pen.setCosmetic(true);
  setPen(pen);
}

int PolyLineItem::pointCount() const
{
  return vertices.count();
}

QPointF PolyLineItem::point(int index) const
{
  if (index < 0 || index >= vertices.count()) {
    return QPointF();
  }
  return vertices[index]->scenePos() - scenePos();
}

void PolyLineItem::setPoint(int index, const QPointF& pos)
{
  if (index < 0 || index >= vertices.count()) {
    qWarning("Point index out of bounds: %d", index);
    return;
  }
  QGraphicsItem* parent = vertices[index]->parentItem();
  QPointF offset(0, 0);
  if (parent) {
    offset = parent->scenePos();
  }
  vertices[index]->setPos(pos - offset);
  updatePolygon();
}

GripItem* PolyLineItem::addPoint(const QPointF& pos)
{
  GripItem* vertex = new GripItem(this);
  vertex->setPos(pos - scenePos());
  addPoint(vertex);
  return vertex;
}

void PolyLineItem::addPoint(GripItem* vertex)
{
  QObject::connect(vertex, SIGNAL(moved(GripItem*, QPointF)), this, SLOT(updatePolygon()));
  vertices << vertex;
  updatePolygon();
}

void PolyLineItem::updatePolygon()
{
  QPolygonF poly;
  for (GripItem* vertex : vertices) {
    poly << (vertex->scenePos() - scenePos());
  }
  setPolygon(poly);
}

GripItem* PolyLineItem::grip(int index) const
{
  if (index < 0 || index >= vertices.count()) {
    qWarning("Edge index out of bounds: %d", index);
    return nullptr;
  }
  return vertices[index];
}

void PolyLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  int numVertices = vertices.length();
  if (numVertices < 2) {
    return;
  }

  painter->setPen(pen());
  painter->drawPolyline(polygon());

  QPointF offset = scenePos();
  QPointF firstPoint = vertices[0]->scenePos() - offset;
  QPointF lastPoint = vertices.last()->scenePos() - offset;;

  QPen p = pen();
  p.setDashPattern({ 1, 4 });
  p.setCapStyle(Qt::FlatCap);
  painter->setPen(p);
  painter->drawLine(lastPoint, firstPoint);
}
