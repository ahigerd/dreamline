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
  bool hasGrip(GripItem* grip) const;

  QPainterPath shape() const;
  void updateShape();

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
