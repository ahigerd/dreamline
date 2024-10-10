#include "edgeitem.h"
#include "gripitem.h"
#include "meshitem.h"
#include "mathutil.h"
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsPathItem>
#include <QPen>
#include <QPainter>

EdgeItem::EdgeItem(GripItem* left, GripItem* right)
: QObject(nullptr), QGraphicsLineItem(QLineF(left->pos(), right->pos()), left->parentItem()), left(left), right(right)
{
  setAcceptHoverEvents(true);
  hoverLeaveEvent(nullptr);
  setZValue(0.1);

  QObject::connect(left, SIGNAL(moved(GripItem*, QPointF)), this, SLOT(updateVertices()));
  QObject::connect(right, SIGNAL(moved(GripItem*, QPointF)), this, SLOT(updateVertices()));
}

QPainterPath EdgeItem::shape() const
{
  QPainterPath path;

  QLineF normVector = line().normalVector();
  normVector.setLength(2);
  QPointF norm = normVector.p2() - normVector.p1();

  QLineF capVector = line();
  capVector.setLength(3.5);
  QPointF cap = capVector.p2() - capVector.p1();

  QPointF p1 = mapFromItem(parentItem(), line().p1());
  QPointF p2 = mapFromItem(parentItem(), line().p2());
  path.moveTo(p1 - norm);
  path.lineTo(p1 + norm);
  path.lineTo(p2 + norm);
  path.lineTo(p2 - norm);
  path.closeSubpath();
  return path;
}

void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  MeshItem* mesh = dynamic_cast<MeshItem*>(parentItem());
  if (mesh && !mesh->edgesVisible()) {
    return;
  }
  QPen p = pen();
  p.setCosmetic(true);
  painter->setPen(p);
  painter->drawLine(line());
}

void EdgeItem::hoverEnter()
{
  QPen pen(Qt::black, 3);
  pen.setCosmetic(true);
  setPen(pen);
}

void EdgeItem::hoverLeave()
{
  QPen pen(Qt::black, 0);
  pen.setCosmetic(true);
  setPen(pen);
}

void EdgeItem::split(const QPointF& pos)
{
  emit insertVertex(this, pos);
}

QColor EdgeItem::colorAt(const QPointF& pos) const
{
  float t = QLineF(pos, right->pos()).length() / line().length();
  QColor leftColor = left->color();
  QColor rightColor = right->color();
  QColor newColor = lerp(leftColor, rightColor, t);
  return newColor;
}

void EdgeItem::updateVertices()
{
  setLine(QLineF(left->pos(), right->pos()));
}

EdgeItem* EdgeItem::split(GripItem* newVertex)
{
  EdgeItem* other = new EdgeItem(newVertex, right);
  QObject::disconnect(right, nullptr, this, nullptr);
  QObject::connect(newVertex, SIGNAL(moved(GripItem*, QPointF)), this, SLOT(updateVertices()));
  right = newVertex;
  updateVertices();
  return other;
}

bool EdgeItem::hasGrip(GripItem* grip) const
{
  return grip == left || grip == right;
}

QPointF EdgeItem::nearestPointOnLine(const QPointF& point)
{
  QLineF norm = line().normalVector();
  norm.setPoints(point, point + QPointF(norm.dx(), norm.dy()));
  QPointF result;
  norm.intersects(line(), &result);

  // Check if the result is outside the bounds of the line segment
  if (QLineF(line().center(), result).length() > line().length() / 2) {
    // If the point is outside, snap to the nearest endpoint
    if (QLineF(line().p1(), result).length() > QLineF(line().p2(), result).length()) {
      result = line().p2();
    } else {
      result = line().p1();
    }
  }
  return result;
}
