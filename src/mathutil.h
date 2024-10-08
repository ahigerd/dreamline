#ifndef DL_MATHUTIL_H
#define DL_MATHUTIL_H

#include <QColor>
#include <QPointF>

double lerp(double a, double b, double t);
QColor lerp(const QColor& a, const QColor& b, double t);

double signedAngle(const QPointF& a, const QPointF& b, const QPointF& c);
double ccwAngle(const QPointF& a, const QPointF& b, const QPointF& c);

#endif
