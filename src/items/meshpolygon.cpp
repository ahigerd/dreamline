#include "meshpolygon.h"
#include "meshitem.h"
#include "meshrenderdata.h"
#include "gripitem.h"
#include "edgeitem.h"
#include "mathutil.h"
#include <QTextStream>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath>

MeshPolygon::MeshPolygon(MeshItem* owner)
: windingDirection(0), owner(owner)
{
  // initializers only
}

bool MeshPolygon::insertVertex(GripItem* vertex, EdgeItem* oldEdge, EdgeItem* newEdge)
{
  if (!edges.contains(oldEdge)) {
    return false;
  }

  edges.append(newEdge);
  GripItem* p1 = oldEdge->leftGrip();
  GripItem* p2 = newEdge->rightGrip();

  int len = vertices.length();
  for (int i = 0; i < len; i++) {
    GripItem* pp1 = vertices[i];
    GripItem* pp2 = vertices[(i + 1) % len];
    if ((pp1 == p1 && pp2 == p2) || (pp1 == p2 && pp2 == p1)) {
      vertices.insert(i + 1, vertex);
      rebuildBuffers();
      return true;
    }
  }

  qWarning("XXX: inconsistent polygon");
  return false;
}

void MeshPolygon::setVertex(int index, const QPointF& pos)
{
  QVector2D v(pos);
  vertexBuffer[index] = v;
  if (vertices[index]->isSmooth()) {
    int numVertices = vertices.count();
    QPolygonF poly = owner->renderData()->rawBoundary;
    int boundIndex = poly.indexOf(pos);
    if (boundIndex >= 0) {
      // The point is one of the boundary control points
      int numBound = poly.count();
      QPointF left = vertices[(index + numVertices - 1) % numVertices]->pos();
      QPointF lastPos = poly[(boundIndex + numBound - (int)windingDirection) % numBound];
      QPointF midpoint1 = (pos + left) / 2.0;
      double leftAngle = smallAngle(left, pos, lastPos);
      QPointF right = vertices[(index + 1) % numVertices]->pos();
      QPointF nextPos = poly[(boundIndex + numBound + (int)windingDirection) % numBound];
      QPointF midpoint2 = (pos + right) / 2.0;
      double rightAngle = smallAngle(right, pos, nextPos);
      qDebug() << pos << leftAngle << rightAngle << midpoint1 << midpoint2;
      QPointF v1 = midpoint1 - pos;
      QPointF v2 = midpoint2 - pos;
      if ((rightAngle < 0.1) == (leftAngle >= 0.1)) {
        QPointF p1 = midpoint1;
        QPointF p2 = midpoint2;
        double a1 = 0;
        double a2 = M_PI_2;
        QLineF edge(pos, left);
        QPointF p;
        while ((a2 - a1) > 0.001) {
          double t = (a1 + a2) / 2;
          p = ellipsePos(v1, v2, pos, t);
          if (QLineF(p1, p).intersects(edge, nullptr) == QLineF::BoundedIntersection) {
            p2 = p;
            a2 = t;
          } else {
            p1 = p;
            a1 = t;
          }
        }
        v = QVector2D((p1 + p2) / 2);
      } else {
        v = QVector2D(ellipsePos(v1, v2, pos, M_PI_4));
      }
      qDebug() << "adj" << pos << index << v;
    }
  }
  colorPoints[index] = v;
}

QColor MeshPolygon::color(int index) const
{
  return QColor::fromRgbF(colors[index][0], colors[index][1], colors[index][2], colors[index][3]);
}

void MeshPolygon::setColor(int index, const QColor& color)
{
  colors[index] = QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void MeshPolygon::updateWindingDirection()
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

void MeshPolygon::rebuildBuffers()
{
  int numVertices = vertices.length();
  vertexBuffer.resize(numVertices);
  colorPoints.resize(numVertices);
  colors.resize(numVertices);
  for (int i = 0; i < numVertices; i++) {
    GripItem* grip = vertices[i];
    QColor color = grip->color();
    setVertex(i, grip->pos());
    setColor(i, color);
  }
  qDebug() << colorPoints.vector();
  updateWindingDirection();
}

QSet<EdgeItem*> MeshPolygon::edgesContainingVertex(GripItem* vertex) const
{
  QSet<EdgeItem*> result;

  for (EdgeItem* edge : edges) {
    if (edge->hasGrip(vertex)) {
      result += edge;
    }
  }

  return result;
}

bool MeshPolygon::testEdge(GripItem* v1, GripItem* v2, EdgeItem* edge1, EdgeItem* edge2) const
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

bool MeshPolygon::isEdgeInside(GripItem* v1, GripItem* v2) const
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

QString MeshPolygon::debug() const
{
  QString result;
  QTextStream ts(&result, QIODevice::WriteOnly);

  if (windingDirection < 0) {
    ts << "-";
  } else if (windingDirection > 0) {
    ts << "+";
  } else {
    ts << "0";
  }

  ts << "P{";

  int n = vertices.length();
  for (int i = 0; i < n; i++) {
    const GripItem* v = vertices[i];
    QVector4D colorVec = colors[i];
    QColor color = QColor::fromRgbF(colorVec[0], colorVec[1], colorVec[2], colorVec[3]);
    if (i > 0) {
      ts << ",";
    }
    ts << "(" << v->pos().x() << "," << v->pos().y() << color.name() << QStringLiteral("%1").arg(color.alpha(), 2, 16) << ")";
  }

  ts << "}";

  return result;
}
