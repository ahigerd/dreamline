#ifndef DL_MARKERITEM_H
#define DL_MARKERITEM_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QColor>
#include <QPalette>

class MarkerItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT
public:
  MarkerItem(QGraphicsItem* parent = nullptr);

  QColor color() const;

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*);

  void setHighlight(const QColor& color);

public slots:
  void setColor(const QColor& color);

signals:
  void colorChanged(MarkerItem* item, const QColor& pos);

private:
  QColor highlightColor = Qt::white;
};

#endif
