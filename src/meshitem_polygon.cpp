#include "meshitem.h"
#include "gripitem.h"
#include "edgeitem.h"
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath>

// This function returns an angle between -pi and +pi that measures how
// much the vector AB the vector must rotate to align with the vector BC.
// Positive numbers are counter-clockwise.
// Negative numbers are clockwise.
static double signedAngle(const QPointF& a, const QPointF& b, const QPointF& c)
{
  double x1 = b.x() - a.x();
  double y1 = b.y() - a.y();
  double x2 = c.x() - b.x();
  double y2 = c.y() - b.y();
  return std::atan2(y2 * x1 - x2 * y1, x1 * x2 + y1 * y2);
}

// This function returns an angle between 0 and 2*pi that measures the
// counterclockwise angle between vector BA and vector CA.
static double ccwAngle(const QPointF& a, const QPointF& b, const QPointF& c)
{
  double x1 = a.x() - b.x();
  double y1 = a.y() - b.y();
  double x2 = c.x() - b.x();
  double y2 = c.y() - b.y();
  double t = std::atan2(y2 * x1 - x2 * y1, x1 * x2 + y1 * y2);
  return (t < 0) ? t + M_PI * 2 : t;
}

MeshItem::Polygon::Polygon()
: windingDirection(0)
{
  // initializers only
}

bool MeshItem::Polygon::insertVertex(GripItem* vertex, EdgeItem* oldEdge, EdgeItem* newEdge)
{
  if (!edges.contains(oldEdge)) {
    return false;
  }

  edges.append(newEdge);
  GripItem* p1 = oldEdge->leftGrip();
  GripItem* p2 = newEdge->rightGrip();
  QColor color = vertex->color();

  int len = vertices.length();
  QVector<QPointF> vbo = vertexBuffer.vector();
  for (int i = 0; i < len; i++) {
    GripItem* pp1 = vertices[i];
    GripItem* pp2 = vertices[(i + 1) % len];
    if ((pp1 == p1 && pp2 == p2) || (pp1 == p2 && pp2 == p1)) {
      vertices.insert(i + 1, vertex);
      vbo.insert(i + 1, vertex->pos());
      colors.insert(i + 1, QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
      vertexBuffer = vbo;
      return true;
    }
  }

  qWarning("XXX: inconsistent polygon");
  return false;
}

QColor MeshItem::Polygon::color(int index) const
{
  return QColor::fromRgbF(colors[index][0], colors[index][1], colors[index][2], colors[index][3]);
}

void MeshItem::Polygon::setColor(int index, const QColor& color)
{
  colors[index] = QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void MeshItem::Polygon::updateWindingDirection()
{
  windingDirection = 0.0f;
  int n = vertices.length();
  if (n < 3) {
    return;
  }
  QPointF a = vertices[n - 2]->pos();
  QPointF b = vertices[n - 1]->pos();
  QPointF c;
  for (int i = 0; i < n; i++) {
    c = vertices[i]->pos();
    windingDirection += signedAngle(a, b, c);
    a = b;
    b = c;
  }
  windingDirection = windingDirection > 0 ? 1.0f : -1.0f;
}

void MeshItem::Polygon::rebuildBuffers()
{
  QPolygonF poly;
  colors.clear();
  for (GripItem* grip : vertices) {
    poly.append(grip->pos());
    QColor color = grip->color();
    colors.append(QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
  }
  vertexBuffer = poly;
  updateWindingDirection();
}

QSet<EdgeItem*> MeshItem::Polygon::edgesContainingVertex(GripItem* vertex) const
{
  QSet<EdgeItem*> result;

  for (EdgeItem* edge : edges) {
    if (edge->hasGrip(vertex)) {
      result += edge;
    }
  }

  return result;
}

bool MeshItem::Polygon::testEdge(GripItem* v1, GripItem* v2, EdgeItem* edge1, EdgeItem* edge2) const
{
  // Get the vertices on either side of the target vertex.
  GripItem* vertexBefore = edge1->leftGrip() == v1 ? edge1->rightGrip() : edge1->leftGrip();
  GripItem* vertexAfter = edge2->leftGrip() == v1 ? edge2->rightGrip() : edge2->leftGrip();

  // Check the order of the vertices around the winding direction of the polygon.
  // By reversing the order of the vertices for reverse-wound polygons, we can
  // ensure you can always use a counter-clockwise angle to determine inclusion.
  double wind = windingDirection;
  int beforePos = vertices.indexOf(vertexBefore) * wind;
  int vertexPos = vertices.indexOf(v1) * wind;
  int afterPos = vertices.indexOf(vertexAfter) * wind;

  // Because the vertex order wraps around the array, it's possible that the target
  // vertex isn't numerically between the left and right vertices. Take that into
  // account while checking to make sure the vertices are in the correct order.
  // If they're not, switch them around so that they are.
  bool ascending = beforePos < afterPos;
  bool vertexBetween = (beforePos < vertexPos && vertexPos < afterPos) || (afterPos < vertexPos && vertexPos < beforePos);
  if (vertexBetween == ascending) {
    std::swap(vertexBefore, vertexAfter);
  }

  // Once things are arranged correctly, you can tell if the new edge is inside
  // the polygon because it will have less of an angle measured in the winding
  // direction than the existing angle.
  double vertexAngle = ccwAngle(vertexBefore->pos(), v1->pos(), vertexAfter->pos());
  double a1 = ccwAngle(vertexBefore->pos(), v1->pos(), v2->pos());
  return a1 < vertexAngle;
}

bool MeshItem::Polygon::isEdgeInside(GripItem* v1, GripItem* v2) const
{
  if (!windingDirection) {
    // A degenerate polygon can't contain anything
    return false;
  }
  QSet<EdgeItem*> edges1 = edgesContainingVertex(v1);
  QSet<EdgeItem*> edges2 = edgesContainingVertex(v2);
  if (edges1.intersects(edges2)) {
    // v1 and v2 already have a shared edge
    return false;
  }
  if (edges1.size() != 2 || edges2.size() != 2) {
    // This shouldn't be possible, but as a sanity check...
    return false;
  }

  // Is the new edge between the edges adjacent to the first vertex?
  if (!testEdge(v1, v2, *edges1.begin(), *(++edges1.begin()))) {
    return false;
  }

  // Is the new edge between the edges adjacent to the second vertex?
  if (!testEdge(v2, v1, *edges2.begin(), *(++edges2.begin()))) {
    return false;
  }

  return true;
}
