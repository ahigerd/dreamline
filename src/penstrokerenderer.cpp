#include "penstrokerenderer.h"
#include "meshitem.h"
#include "meshrenderdata.h"
#include "gripitem.h"
#include "propertypanel.h"
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#define _USE_MATH_DEFINES
#include <cmath>

/**
 * @param a The vector from the control point to the start of the arc
 * @param b The vector from the control point to the end of the arc
 * @param i The control point (outside of the arc)
 * @param t The angle, between 0 and pi/2
 */
static QPointF ellipseInterpolate(const QPointF& a, const QPointF& b, const QPointF& i, double t)
{
  double s = 1.0 - std::sin(t);
  double c = 1.0 - std::cos(t);
  return i + a * s + b * c;
}

PenStrokeRenderer::PenStrokeRenderer()
: AbstractMeshRenderer()
{
  // initializers only
}

void PenStrokeRenderer::render(MeshItem* mesh, MeshRenderData* data, QPainter* painter, GLFunctions*)
{
  int numVertices = mesh->numBoundaryVertices();
  if (numVertices < 2) {
    return;
  }

  painter->setPen(mesh->strokePen());
  QTransform transform = painter->deviceTransform().inverted();
  double dx = transform.m11();
  double dy = transform.m22();
  double pixelSize = std::sqrt(dx * dx + dy * dy);

  if (data->strokeOwner == this && data->lastStrokeScale == pixelSize) {
    painter->drawPath(data->strokePath);
    return;
  }

  data->strokeOwner = this;
  data->lastStrokeScale = pixelSize;

  QPainterPath path;
  GripItem* grip = mesh->boundaryVertex(numVertices - 1);
  QPointF lastPos = grip->scenePos();
  if (grip->isSmooth()) {
    path.moveTo((mesh->boundaryVertex(0)->scenePos() + lastPos) / 2.0);
  } else {
    path.moveTo(lastPos);
  }
  for (int i = 0; i < numVertices; i++) {
    grip = mesh->boundaryVertex(i);
    QPointF pos = grip->scenePos();
    if (grip->isSmooth()) {
      QPointF nextPos = mesh->boundaryVertex((i + 1) % numVertices)->scenePos();
      QPointF midpoint1 = (pos + lastPos) / 2.0;
      QPointF midpoint2 = (pos + nextPos) / 2.0;
      path.lineTo(midpoint1);
      double radiansPerPixel = pixelSize / (M_PI_2 * (QLineF(pos, midpoint1).length() + QLineF(pos, midpoint2).length()));
      QPointF v1 = midpoint1 - pos;
      QPointF v2 = midpoint2 - pos;
      for (double t = radiansPerPixel; t < M_PI_2; t += radiansPerPixel) {
        path.lineTo(ellipseInterpolate(v1, v2, pos, t));
      }
      path.lineTo(midpoint2);
    } else {
      path.lineTo(grip->scenePos());
    }
    lastPos = pos;
  }
  data->strokePath = path;
  painter->drawPath(path);
}

PenStrokePropertyPanel::PenStrokePropertyPanel()
: PropertyPanel(nullptr)
{
  // initializers only
  setAutoFillBackground(true);
}

void PenStrokePropertyPanel::updateAllProperties()
{
  QPalette p = palette();
  p.setBrush(QPalette::Window, currentMesh()->strokePen().color());
  setPalette(p);
}
