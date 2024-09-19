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

  m_vbo.resize(3);
  m_vbo[0] = QPointF(-100, -100);
  m_vbo[1] = QPointF(100, -50);
  m_vbo[2] = QPointF(0, 100);

  m_colorBuffer = {
    QColor(Qt::red),
    QColor(Qt::green),
    QColor(Qt::blue),
  };

  for (int i = 0; i < m_vbo.count(); i++) {
    GripItem* grip = new GripItem(i, this);
    grip->setPos(m_vbo[i]);
    m_grips.append(grip);
    QObject::connect(grip, SIGNAL(moved(int, QPointF)), this, SLOT(moveVertex(int, QPointF)));
  }
}

void PolygonItem::moveVertex(int id, const QPointF& pos)
{
  m_vbo[id] = pos;
  setPolygon(m_vbo.vector());
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
  program->setAttributeBuffer(1, GL_FLOAT, 0, 4, 4 * sizeof(float));

  QTransform transform = gl->transform();
  program->setUniformValue("translate", transform.dx() + x() * transform.m11(), transform.dy() + y() * transform.m22());
  program->setUniformValue("scale", transform.m11(), transform.m22());

  gl->glEnableVertexAttribArray(0);
  gl->glEnableVertexAttribArray(1);

  gl->glDrawArrays(GL_TRIANGLES, 0, m_vbo.count());
  m_colorBuffer.release();
  m_vbo.release();

  painter->endNativePainting();
}
