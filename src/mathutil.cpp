#include "mathutil.h"

#define _USE_MATH_DEFINES
#include <cmath>

double lerp(double a, double b, double t)
{
  return (a * t) + (b * (1 - t));
}

QColor lerp(const QColor& a, const QColor& b, double t)
{
  return QColor(
    lerp(a.red(), b.red(), t),
    lerp(a.green(), b.green(), t),
    lerp(a.blue(), b.blue(), t),
    lerp(a.alpha(), b.alpha(), t)
  );
}

// This function returns an angle between -pi and +pi that measures how
// much the vector AB the vector must rotate to align with the vector BC.
// Positive numbers are counter-clockwise.
// Negative numbers are clockwise.
double signedAngle(const QPointF& a, const QPointF& b, const QPointF& c)
{
  double x1 = b.x() - a.x();
  double y1 = b.y() - a.y();
  double x2 = c.x() - b.x();
  double y2 = c.y() - b.y();
  return std::atan2(y2 * x1 - x2 * y1, x1 * x2 + y1 * y2);
}

// This function returns an angle between 0 and pi that measures the
// smaller angle from vector BA around point B to vector BC.
double smallAngle(const QPointF& a, const QPointF& b, const QPointF& c)
{
  double x1 = a.x() - b.x();
  double y1 = a.y() - b.y();
  double x2 = c.x() - b.x();
  double y2 = c.y() - b.y();
  double t = std::abs(std::atan2(y2 * x1 - x2 * y1, x1 * x2 + y1 * y2));
  if (t > M_PI) {
    return t - M_PI;
  }
  return t;
}

// This function returns an angle between 0 and 2*pi that measures the
// counterclockwise angle from vector BA around point B to vector BC.
double ccwAngle(const QPointF& a, const QPointF& b, const QPointF& c)
{
  double x1 = a.x() - b.x();
  double y1 = a.y() - b.y();
  double x2 = c.x() - b.x();
  double y2 = c.y() - b.y();
  double t = std::atan2(y2 * x1 - x2 * y1, x1 * x2 + y1 * y2);
  return (t < 0) ? t + M_PI * 2 : t;
}

static inline QPointF scaledNormalVector(const QPointF& p1, const QPointF& p2, double distance)
{
  QLineF norm(0, 0, 0, 0);
  norm.setP2(QPointF(p1.y() - p2.y(), p2.x() - p1.x()));
  norm.setLength(distance);
  return norm.p2();
}

// Assumes counterclockwise winding angles
QPolygonF expandPolygon(const QPolygonF& source, double distance)
{
  QPolygonF result;

  int numVertices = source.length();
  if (numVertices < 1) {
    return result;
  } else if (numVertices == 1) {
    QPointF p = source[0];
    result << (p + QPointF(0, distance * -2));
    // cos(60) = 1/2, sin(60) = sqrt(3)/2
    result << (p + QPointF(distance, distance * 1.7321));
    result << (p + QPointF(-distance, distance * 1.7321));
    return result;
  }

  QPointF p1 = source[source.length() - 1];
  QLineF norm;
  QLineF lastVector, vector;
  QPointF nextPoint;

  bool first = true;
  for (int i = 0; i <= numVertices; i++) {
    QPointF p2 = source[i % numVertices];
    QPointF n = scaledNormalVector(p1, p2, distance);
    if (n.isNull()) {
      continue;
    }
    vector.setPoints(p1 - n, p2 - n);
    if (first) {
      result << ((p1 + p2) / 2 - n);
      first = false;
    } else {
      QPointF control;
      auto intersect = lastVector.intersects(vector, &control);
      if (intersect == QLineF::NoIntersection) {
        continue;
      } else {
        result << control;
      }
    }
    nextPoint = p2 - n;

    lastVector = vector;
    p1 = p2;
  }

  return result;
}

// Assumes counterclockwise winding angles
QPainterPath expandPolygonRounded(const QPolygonF& source, double distance)
{
  QPainterPath result;

  int numVertices = source.length();
  if (numVertices < 1) {
    return result;
  } else if (numVertices == 1) {
    QPointF p = source[0];
    result.addEllipse(p.x() - distance, p.y() - distance, distance * 2, distance * 2);
    return result;
  }

  QPointF p1 = source[source.length() - 1];
  QLineF norm;
  QLineF lastVector, vector;
  QPointF nextPoint;

  bool first = true;
  for (int i = 0; i <= numVertices; i++) {
    QPointF p2 = source[i % numVertices];
    QPointF n = scaledNormalVector(p1, p2, distance);
    if (n.isNull()) {
      continue;
    }
    vector.setPoints(p1 - n, p2 - n);
    if (first) {
      result.moveTo((p1 + p2) / 2 - n);
      first = false;
    } else {
      QPointF control;
      auto intersect = lastVector.intersects(vector, &control);
      if (intersect == QLineF::NoIntersection) {
        continue;
      } else if (intersect == QLineF::BoundedIntersection) {
        result.lineTo(control);
      } else {
        result.lineTo(nextPoint);
        result.cubicTo((lastVector.p2() + control) / 2, (p1 - n + control) / 2, p1 - n);
      }
    }
    nextPoint = p2 - n;

    lastVector = vector;
    p1 = p2;
  }
  result.closeSubpath();

  return result;
}

/**
 * @param a The vector from the control point to the start of the arc
 * @param b The vector from the control point to the end of the arc
 * @param i The control point (outside of the arc)
 * @param t The pre-transformation angle, between 0 and pi/2
 * @return The point on the ellipse at the requested angle
 */
QPointF ellipsePos(const QPointF& a, const QPointF& b, const QPointF& i, double t)
{
  double s = 1.0 - std::sin(t);
  double c = 1.0 - std::cos(t);
  return i + a * s + b * c;
}
