#include "dreamproject.h"
#include "polygonitem.h"
#include <QGraphicsRectItem>
#include <QPalette>
#include <QPainter>
#include <QtDebug>

#define DPI 100

DreamProject::DreamProject(const QSizeF& pageSize, QObject* parent)
: QGraphicsScene(parent)
{
  setBackgroundBrush(QColor(139,134,128,255));

  setPageSize(pageSize);

  // TODO: A new project should remain blank, but this is convenient for testing.
  PolygonItem* p = new PolygonItem;
  addItem(p);
}

QSizeF DreamProject::pageSize() const
{
  return pageRect.size();
}

void DreamProject::setPageSize(const QSizeF& size)
{
  double width = DPI * size.width();
  double height = DPI * size.height();
  pageRect = QRectF(-(width / 2), -(height / 2), width, height);
  setSceneRect(pageRect.adjusted(-DPI, -DPI, DPI, DPI));
}

void DreamProject::drawBackground(QPainter* p, const QRectF& rect)
{
  p->fillRect(rect, backgroundBrush());

  p->setBrush(Qt::white);
  p->setPen(QPen(Qt::black));
  p->drawRect(pageRect);
}
