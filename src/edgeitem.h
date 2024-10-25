#ifndef DL_EDGEITEM_H
#define DL_EDGEITEM_H

#include <QGraphicsLineItem>
#include <QObject>
class GripItem;
class QGraphicsPathItem;

class EdgeItem : public QObject, public QGraphicsLineItem
{
Q_OBJECT
public:
  EdgeItem(GripItem* left, GripItem* right);

  EdgeItem* split(GripItem* newVertex);

  inline GripItem* leftGrip() const { return left; }
  inline GripItem* rightGrip() const { return right; }
  inline GripItem* otherGrip(GripItem* grip) const { return left == grip ? right : left; }
  bool hasGrip(GripItem* grip) const;
  QPointF nearestPointOnLine(const QPointF& point);

  QPainterPath shape() const;
  void updateShape();
  void split(const QPointF& pos);

  QColor colorAt(const QPointF& pos) const;

  void hoverEnter();
  void hoverLeave();

signals:
  void insertVertex(EdgeItem*, const QPointF&);

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

protected slots:
  void updateVertices();

private:
  GripItem* left;
  GripItem* right;
};

#endif
