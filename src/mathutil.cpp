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
