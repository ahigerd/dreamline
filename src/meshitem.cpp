#include "meshitem.h"
#include "glviewport.h"
#include "gripitem.h"
#include "edgeitem.h"
#include <QOpenGLVertexArrayObject>
#include <QPainter>
#include <QFile>

MeshItem::MeshItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsPolygonItem(parent)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);

  QPolygonF poly({
    QPointF(-100, -100),
    QPointF(100, -100),
    QPointF(100, 100),
    QPointF(-100, 100),
  });
  setPolygon(poly);

  Polygon polygonData;
  polygonData.vertexBuffer = QPolygonF({ poly[0], poly[1], poly[2] });
  polygonData.colors = {
    { 1.0, 0.0, 0.0, 1.0 },
    { 0.0, 1.0, 0.0, 1.0 },
    { 0.0, 0.0, 1.0, 1.0 },
  };

  Polygon polygonData2;
  polygonData2.vertexBuffer = QPolygonF({ poly[2], poly[0], poly[3] });
  polygonData2.colors = {
    { 0.0, 0.0, 1.0, 1.0 },
    { 1.0, 0.0, 0.0, 1.0 },
    { 1.0, 1.0, 0.0, 1.0 },
  };

  m_polygons.append(polygonData);
  m_polygons.append(polygonData2);

  for (Polygon& p : m_polygons) {
    int numVertices = p.vertexBuffer.count();
    for (int i = 0; i < numVertices; i++) {
      GripItem* grip = nullptr;
      for (GripItem* checkGrip : m_grips) {
        if (checkGrip->pos() == p.vertexBuffer[i]) {
          grip = checkGrip;
          break;
        }
      }
      if (!grip) {
        grip = new GripItem(this);
        m_grips.append(grip);
        m_boundary.append(grip);
        grip->setPos(p.vertexBuffer[i]);
        grip->setBrush(p.color(i));
        QObject::connect(grip, SIGNAL(moved(GripItem*, QPointF)), this, SLOT(moveVertex(GripItem*, QPointF)));
        QObject::connect(grip, SIGNAL(colorChanged(GripItem*, QColor)), this, SLOT(changeColor(GripItem*, QColor)));
      }
      p.vertices.append(grip);
    }
    for (int i = 0; i < numVertices; i++) {
      GripItem* left = p.vertices[i];
      GripItem* right = p.vertices[(i + 1) % numVertices];
      EdgeItem* edge = nullptr;
      for (EdgeItem* checkEdge : m_edges) {
        if (checkEdge->hasGrip(left) && checkEdge->hasGrip(right)) {
          edge = checkEdge;
          break;
        }
      }
      if (!edge) {
        edge = new EdgeItem(left, right);
        m_edges.append(edge);
        QObject::connect(edge, SIGNAL(insertVertex(EdgeItem*,QPointF)), this, SLOT(insertVertex(EdgeItem*,QPointF)));
      }
      p.edges.append(edge);
    }
  }
}

void MeshItem::moveVertex(GripItem* vertex, const QPointF& pos)
{
  int boundaryIndex = m_boundary.indexOf(vertex);
  if (boundaryIndex >= 0) {
    QPolygonF p = polygon();
    p[boundaryIndex] = pos;
    setPolygon(p);
  }

  for (Polygon& poly : m_polygons) {
    int index = poly.vertices.indexOf(vertex);
    if (index >= 0) {
      poly.vertexBuffer[index] = pos;
    }
  }
}

void MeshItem::changeColor(GripItem* vertex, const QColor& color)
{
  for (Polygon& poly : m_polygons) {
    int index = poly.vertices.indexOf(vertex);
    if (index >= 0) {
      poly.colors[index] = QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
  }
}

void MeshItem::insertVertex(EdgeItem* edge, const QPointF& pos)
{
  int oldIndex = m_edges.indexOf(edge);
  if (oldIndex < 0) {
    qDebug() << "XXX: unknown edge";
    return;
  }

  GripItem* p1 = edge->leftGrip();
  GripItem* p2 = edge->rightGrip();

  GripItem* grip = new GripItem(this);
  grip->setPos(pos);
  QObject::connect(grip, SIGNAL(moved(GripItem*, QPointF)), this, SLOT(moveVertex(GripItem*, QPointF)));
  QObject::connect(grip, SIGNAL(colorChanged(GripItem*, QColor)), this, SLOT(changeColor(GripItem*, QColor)));
  m_grips.append(grip);

  float t = QLineF(pos, p2->pos()).length() / edge->line().length();
  QColor leftColor = p1->color();
  QColor rightColor = p2->color();
  QColor newColor(
    (leftColor.red() * t) + (rightColor.red() * (1.0f - t)),
    (leftColor.green() * t) + (rightColor.green() * (1.0f - t)),
    (leftColor.blue() * t) + (rightColor.blue() * (1.0f - t))
  );
  grip->setColor(newColor);

  EdgeItem* newEdge = edge->split(grip);
  QObject::connect(newEdge, SIGNAL(insertVertex(EdgeItem*,QPointF)), this, SLOT(insertVertex(EdgeItem*,QPointF)));
  m_edges.append(newEdge);

  int numRefs = 0;
  for (Polygon& poly : m_polygons) {
    bool ref = poly.insertVertex(grip, edge, newEdge);
    if (ref) {
      numRefs++;
    }
  }

  if (numRefs == 1) {
    // A singly-referenced edge is an exterior edge
    QPolygonF p = polygon();
    int len = p.length();
    bool found = false;
    for (int i = 0; i < len; i++) {
      GripItem* pp1 = m_boundary[i];
      GripItem* pp2 = m_boundary[(i + 1) % len];
      if ((pp1 == p1 && pp2 == p2) || (pp1 == p2 && pp2 == p1)) {
        p.insert(i + 1, pos);
        m_boundary.insert(i + 1, grip);
        found = true;
        break;
      }
    }
    if (!found) {
      qWarning("XXX: insertion point not found");
    }
    setPolygon(p);
  }
}

void MeshItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  painter->beginNativePainting();

  GLViewport* gl = GLViewport::instance(QOpenGLContext::currentContext());
  if (!gl) {
    painter->endNativePainting();
    qFatal("no context");
    return;
  }

  gl->glEnable(GL_BLEND);
  for (Polygon& poly : m_polygons) {
    GLBuffer<QPointF>& vbo = poly.vertexBuffer;
    BoundProgram program = gl->useShader("polyramp", vbo.count());

    program.bindAttributeBuffer(0, vbo);

    QVector<QVector2D> verts(vbo.count());
    for (int i = 0; i < vbo.count(); i++) {
      verts[i] = QVector2D(vbo[i]);
    }
    program->setUniformValueArray("verts", verts.constData(), verts.size());
    program->setUniformValueArray("colors", poly.colors.constData(), poly.colors.size());

    QTransform transform = gl->transform();
    program->setUniformValue("translate", transform.dx() + x() * transform.m11(), transform.dy() + y() * transform.m22());
    program->setUniformValue("scale", transform.m11(), transform.m22());

    gl->glDrawArrays(GL_TRIANGLE_FAN, 0, vbo.count());
  }

  painter->endNativePainting();
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
