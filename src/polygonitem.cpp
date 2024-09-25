#include "polygonitem.h"
#include "glviewport.h"
#include "gripitem.h"
#include "edgeitem.h"
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
    grip->setBrush(m_colorBuffer[i]);
    m_grips.append(grip);
    QObject::connect(grip, SIGNAL(moved(int, QPointF)), this, SLOT(moveVertex(int, QPointF)));
    QObject::connect(grip, SIGNAL(colorChanged(int, QColor)), this, SLOT(changeColor(int, QColor)));
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

void PolygonItem::changeColor(int id, const QColor& color)
{
  m_colorBuffer[id] = color;
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
  QObject::connect(grip, SIGNAL(colorChanged(int, QColor)), this, SLOT(changeColor(int, QColor)));
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
  grip->setBrush(newColor);
  colors.insert(index, newColor);
  m_colorBuffer = colors;

  QPolygonF p = polygon();
  p.insert(index, pos);
  setPolygon(p);
  m_vbo = p;
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

  bool isPoly = m_vbo.count() > 3;

  BoundProgram program = isPoly ? gl->useShader("polyramp", m_vbo.count()) : gl->useShader("ramp");

  program.bindAttributeBuffer(0, m_vbo);

  QVector<QVector2D> verts(m_vbo.count());
  for (int i = 0; i < m_vbo.count(); i++) {
    verts[i] = QVector2D(m_vbo[i].x(), m_vbo[i].y());
  }
  program->setUniformValueArray("verts", verts.constData(), verts.size());

  if (isPoly) {
    // TODO: Is it possible to bind a buffer to a uniform?
    // Or are VBOs and UBOs fundamentally different objects?
    QVector<QVector4D> colors(m_colorBuffer.count());
    for (int i = 0; i < m_colorBuffer.count(); i++) {
      colors[i] = QVector4D(m_colorBuffer[i].redF(), m_colorBuffer[i].greenF(), m_colorBuffer[i].blueF(), m_colorBuffer[i].alphaF());
    }
    program->setUniformValueArray("colors", colors.constData(), colors.size());
  } else {
    program.bindAttributeBuffer(1, m_colorBuffer);
  }

  QTransform transform = gl->transform();
  program->setUniformValue("translate", transform.dx() + x() * transform.m11(), transform.dy() + y() * transform.m22());
  program->setUniformValue("scale", transform.m11(), transform.m22());

  gl->glDrawArrays(GL_POLYGON, 0, m_vbo.count());
  if (isPoly) {
    m_colorBuffer.release();
  }
  m_vbo.release();

  painter->endNativePainting();
}
