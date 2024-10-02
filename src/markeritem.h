#ifndef DL_MARKERITEM_H
#define DL_MARKERITEM_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QColor>

class MarkerItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT
public:
  MarkerItem(QGraphicsItem* parent = nullptr);

  QColor color() const;

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*);

public slots:
  void setColor(const QColor& color);

signals:
  void colorChanged(MarkerItem* item, const QColor& pos);
};

#endif
