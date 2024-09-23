#include "polygonitem.h"
#include "glviewport.h"
#include "gripitem.h"
#include "edgeitem.h"
#include <QOpenGLVertexArrayObject>
#include <QPainter>
#include <QFile>
#include <cmath>
#include <QStyleOptionGraphicsItem>

PolygonItem::PolygonItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsPolygonItem(parent)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setCacheMode(QGraphicsItem::DeviceCoordinateCache);

  m_vbo.resize(3);
  m_vbo[0] = QPointF(-100, -100);
  m_vbo[1] = QPointF(100, -50);
  m_vbo[2] = QPointF(0, 100);
  setPolygon(m_vbo.vector());

  m_colorBuffer = {
    QColor(Qt::red),
    QColor(Qt::green),
    QColor(Qt::blue),
  };

  int numVertices = m_vbo.count();

  for (int i = 0; i < numVertices; i++) {
    GripItem* grip = new GripItem(i, this);
    grip->setPos(m_vbo[i]);
    m_grips.append(grip);
    QObject::connect(grip, SIGNAL(moved(int, QPointF)), this, SLOT(moveVertex(int, QPointF)));
  }

  for (int i = 0; i < numVertices; i++) {
    GripItem* left = m_grips[i];
    GripItem* right = m_grips[(i + 1) % numVertices];
    EdgeItem* edge = new EdgeItem(left, right);
    m_edges.append(edge);
    QObject::connect(edge, SIGNAL(insertVertex(EdgeItem*,QPointF)), this, SLOT(insertVertex(EdgeItem*,QPointF)));
  }
}

void PolygonItem::moveVertex(int id, const QPointF& pos)
{
  m_vbo[id] = pos;
  setPolygon(m_vbo.vector());
  int numEdges = m_edges.length();
  EdgeItem* right = m_edges[id];
  right->setLine(QLineF(pos, right->line().p2()));
  EdgeItem* left = m_edges[(id + numEdges - 1) % numEdges];
  left->setLine(QLineF(left->line().p1(), pos));
}

void PolygonItem::insertVertex(EdgeItem* edge, const QPointF& pos)
{
  int oldIndex = m_edges.indexOf(edge);
  if (oldIndex < 0) {
    qDebug() << "XXX: unknown edge";
    return;
  }

  int numVertices = m_grips.length();
  int index = (oldIndex + 1) % numVertices;
  int nextIndex = (index + 1) % numVertices;

  GripItem* grip = new GripItem(index, this);
  grip->setPos(pos);
  QObject::connect(grip, SIGNAL(moved(int, QPointF)), this, SLOT(moveVertex(int, QPointF)));
  m_grips.insert(index, grip);
  for (int i = index + 1; i <= numVertices; i++) {
    m_grips[i]->reindex(i);
  }

  EdgeItem* oldEdge = m_edges[oldIndex];
  QLineF oldLine = oldEdge->line();
  oldEdge->setLine(QLineF(oldLine.p1(), pos));

  EdgeItem* newEdge = new EdgeItem(grip, m_grips[nextIndex]);
  newEdge->setLine(QLineF(pos, oldLine.p2()));
  QObject::connect(newEdge, SIGNAL(insertVertex(EdgeItem*,QPointF)), this, SLOT(insertVertex(EdgeItem*,QPointF)));
  m_edges.insert(index, newEdge);

  float t = QLineF(pos, oldLine.p2()).length() / oldLine.length();
  QColor leftColor = m_colorBuffer[oldIndex];
  QColor rightColor = m_colorBuffer[index];
  QColor newColor(
    (leftColor.red() * t) + (rightColor.red() * (1.0f - t)),
    (leftColor.green() * t) + (rightColor.green() * (1.0f - t)),
    (leftColor.blue() * t) + (rightColor.blue() * (1.0f - t))
  );
  QVector<QColor> colors = m_colorBuffer.vector();
  colors.insert(index, newColor);
  m_colorBuffer = colors;

  QPolygonF p = polygon();
  p.insert(index, pos);
  setPolygon(p);
  m_vbo = p;
}

#if 0
void PolygonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  painter->beginNativePainting();

  GLViewport* gl = GLViewport::instance(QOpenGLContext::currentContext());
  if (!gl) {
    painter->endNativePainting();
    qFatal("no context");
    return;
  }

  BoundProgram program = gl->useShader("ramp");

  program->enableAttributeArray(0);
  m_vbo.bind();
  program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));

  program->enableAttributeArray(1);
  m_colorBuffer.bind();
  program->setAttributeBuffer(1, GL_FLOAT, 0, 4, 4 * sizeof(float));

  QTransform transform = gl->transform();
  program->setUniformValue("translate", transform.dx() + x() * transform.m11(), transform.dy() + y() * transform.m22());
  program->setUniformValue("scale", transform.m11(), transform.m22());

  gl->glEnableVertexAttribArray(0);
  gl->glEnableVertexAttribArray(1);

  gl->glDrawArrays(GL_TRIANGLE_FAN, 0, m_vbo.count());
  m_colorBuffer.release();
  m_vbo.release();

  painter->endNativePainting();
}
#endif

static double cr2(const QPointF& a, const QPointF& b)
{
  return a.x() * b.y() - a.y() * b.x();
}

static double ang(const QPointF& a, const QPointF& b)
{
  return std::atan2(cr2(a, b), QPointF::dotProduct(a, b));
}

void PolygonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* opt, QWidget*)
{
  QImage img(opt->rect.size(), QImage::Format_ARGB32);
  img.fill(0);
  int w = img.width();
  int h = img.height();
  int dx = opt->rect.x();
  int dy = opt->rect.y();

  QPolygonF poly = polygon();
  QPointF prev, curr, next, norm;
  QPointF last3 = poly[poly.length() - 3];
  QPointF last2 = poly[poly.length() - 2];
  QPointF last1 = poly[poly.length() - 1];

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      QPointF pt(x + dx, y + dy);
      if (!poly.containsPoint(pt, Qt::OddEvenFill)) continue;
      double r = 0;
      double g = 0;
      double b = 0;
      double a = 0;
      double t = 0;
      prev = last3;
      curr = last2;
      next = last1;
      for (int i = 0; i < poly.length(); i++) {
        norm = curr - pt;
        double len = std::sqrt(norm.x() * norm.x() + norm.y() * norm.y());
        norm *= 1.0 / len;

        double a1 = ang(prev - pt, norm);
        double a2 = ang(norm, next - pt);
        double t1 = std::tan(a1 * 0.5);
        double t2 = std::tan(a2 * 0.5);
        double w = (t1 + t2) / len;
        t += w;

        QColor c = m_colorBuffer[(i + poly.length() - 2) % poly.length()];
        t += w;
        r += c.red() * w;
        g += c.green() * w;
        b += c.blue() * w;
        a += c.alpha() * w;

        prev = curr;
        curr = next;
        next = poly[i];
      }
      t *= 0.5;
      img.setPixel(x, y, qRgba(r / t, g / t, b / t, a / t));
    }
  }

  painter->drawImage(dx, dy, img);
}
