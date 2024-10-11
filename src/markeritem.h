#ifndef DL_MARKERITEM_H
#define DL_MARKERITEM_H

#include <QGraphicsRectItem>
#include <QObject>

class MarkerItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT
public:
  MarkerItem(QGraphicsItem* parent = nullptr);

  QColor color() const;
  bool isSmooth() const;

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*);

  void setHighlight(const QColor& color);

public slots:
  void setColor(const QColor& color);
  void setSmooth(bool on);

signals:
  void colorChanged(MarkerItem* item, const QColor& pos);
  void smoothChanged(MarkerItem* item, bool smooth);

private:
  void drawFrame(QPainter* painter, const QRectF& rect);
  QColor highlightColor = Qt::white;

  bool m_smooth;
};

#endif
