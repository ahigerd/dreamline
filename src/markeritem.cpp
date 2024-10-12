#include "markeritem.h"
#include "dreamproject.h"
#include <QStyleOptionGraphicsItem>
#include <QColor>
#include <QPen>
#include <QPainter>

MarkerItem::MarkerItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsRectItem(-3.5, -3.5, 7, 7, parent), m_smooth(false)
{
  setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
  setZValue(1);
  QPen pen(Qt::black, 1);
  pen.setCosmetic(true);
  setPen(pen);
  setSmooth(m_smooth);
}

/**
 * @brief Get the current color of the marker
 *
 * @return QColor
 */
QColor MarkerItem::color() const
{
  return brush().color();
}

/**
 * @brief Set the fill color of the marker outline
 */
void MarkerItem::setHighlight(const QColor& color)
{
  highlightColor = color;
}

/**
 * @brief Color the vertex and marker center
 *
 * Sets both the color of the corrisponding vertex in MeshItem
 * and the center color of the MarkerItem
 */
void MarkerItem::setColor(const QColor& color)
{
  setBrush(color);
  emit colorChanged(this, color);
}

bool MarkerItem::isSmooth() const
{
  return m_smooth;
}

void MarkerItem::setSmooth(bool on)
{
  m_smooth = on;
  update();
  emit smoothChanged(this, on);
}

void MarkerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
  if (DreamProject* project = dynamic_cast<DreamProject*>(scene())) {
    if (project->isExporting()) {
      return;
    }
  }
  // Snap coordinates to device pixels to avoid fuzzy edges
  double xFrac = painter->deviceTransform().dx();
  xFrac = xFrac - int(xFrac);

  double yFrac = painter->deviceTransform().dy();
  yFrac = yFrac - int(yFrac);

  QRectF r = rect().adjusted(-xFrac, -yFrac, -xFrac, -yFrac);
  QPen p = pen();
  p.setWidth(3);
  painter->setPen(p);
  painter->setBrush(brush());
  drawFrame(painter, r.adjusted(-1, -1, 1, 1));
  if (option->state & QStyle::State_Selected) {
    p.setColor(option->palette.color(QPalette::Highlight));
  } else {
    p.setColor(highlightColor);
  }
  p.setWidthF(m_smooth ? 1.6 : 1.0);
  painter->setPen(p);
  painter->setBrush(Qt::transparent);
  drawFrame(painter, r.adjusted(-1, -1, 1, 1));
}

void MarkerItem::drawFrame(QPainter* painter, const QRectF& rect)
{
  if (m_smooth) {
    painter->drawEllipse(rect);
  } else {
    painter->drawRect(rect);
  }
}
