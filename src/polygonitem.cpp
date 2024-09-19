#include "polygonitem.h"
#include "gripitem.h"
#include "glviewport.h"
#include <QOpenGLVertexArrayObject>
#include <QPainter>
#include <QFile>

PolygonItem::PolygonItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsPolygonItem(parent)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);

  QPolygonF p;
  p.append(QPointF(-100, -100));
  p.append(QPointF(100, -50));
  p.append(QPointF(0, 100));
  m_vbo = p;
  setPolygon(p);

  m_colorBuffer = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
    /*
    QColor(Qt::red),
    QColor(Qt::green),
    QColor(Qt::blue),
    */
  };

  for (int i = 0; i < p.length(); i++) {
    GripItem* grip = new GripItem(i, this);
    grip->setPos(p[i]);
    m_grips.append(grip);
    QObject::connect(grip, SIGNAL(moved(int, QPointF)), this, SLOT(moveVertex(int, QPointF)));
  }
}

void PolygonItem::moveVertex(int id, const QPointF& pos)
{
  QPolygonF p = polygon();
  p[id] = pos;
  m_vbo = p;
  setPolygon(p);
}

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
  program->setAttributeBuffer(1, GL_FLOAT, 0, 3, 3 * sizeof(float));

  QTransform transform = gl->transform();
  program->setUniformValue("translate", transform.dx(), transform.dy());
  program->setUniformValue("scale", transform.m11(), transform.m22());

  gl->glEnableVertexAttribArray(0);
  gl->glEnableVertexAttribArray(1);

  gl->glDrawArrays(GL_TRIANGLES, 0, m_vbo.count());
  m_colorBuffer.release();
  m_vbo.release();

  painter->endNativePainting();
}
