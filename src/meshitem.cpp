#include "meshitem.h"
#include "glviewport.h"
#include "gripitem.h"
#include "edgeitem.h"
#include <QOpenGLVertexArrayObject>
#include <QPainter>
#include <QFile>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath>

static double signedAngle(const QPointF& a, const QPointF& b, const QPointF& c)
{
  double t1 = std::atan2(a.y() - b.y(), a.x() - b.x());
  double t2 = std::atan2(c.y() - b.y(), c.x() - b.x());
  double t = t1 - t2;
  // explicitly ensure that the result is between -pi and +pi
  while (t < -M_PI) {
    t += M_PI * 2;
  }
  while (t > M_PI) {
    t -= M_PI * 2;
  }
  if (t == 0.0) {
    return M_PI;
  }
  return t;
}

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
        grip = newGrip();
        m_boundary.append(grip);
        grip->setPos(p.vertexBuffer[i]);
        grip->setBrush(p.color(i));
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

  m_lastVertexFocus = new QGraphicsEllipseItem(-8.5, -8.5, 17, 17, this);
  QPen pen(Qt::black, 3);
  pen.setCosmetic(true);
  m_lastVertexFocus->setPen(pen);
  QGraphicsEllipseItem* inner = new QGraphicsEllipseItem(-8.5, -8.5, 17, 17, m_lastVertexFocus);
  pen.setColor(Qt::white);
  pen.setWidth(1.5);
  inner->setPen(pen);
  m_lastVertexFocus->setZValue(100);
  m_lastVertexFocus->hide();
}

GripItem* MeshItem::newGrip()
{
  GripItem* grip = new GripItem(this);
  m_grips.append(grip);
  QObject::connect(grip, SIGNAL(moved(GripItem*, QPointF)), this, SLOT(moveVertex(GripItem*, QPointF)));
  QObject::connect(grip, SIGNAL(colorChanged(GripItem*, QColor)), this, SLOT(changeColor(GripItem*, QColor)));
  QObject::connect(grip, SIGNAL(destroyed(QObject*)), this, SLOT(gripDestroyed(QObject*)));
  return grip;
}

QSet<MeshItem::Polygon*> MeshItem::polygonsContainingVertex(GripItem* vertex)
{
  QSet<Polygon*> result;

  for (Polygon& poly : m_polygons) {
    if (poly.vertices.contains(vertex)) {
      result += &poly;
    }
  }

  return result;
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
      poly.updateWindingDirection();
    }
  }

  if (vertex == m_lastVertex) {
    m_lastVertexFocus->setPos(pos);
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

  GripItem* grip = newGrip();
  grip->setPos(pos);

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

  setActiveVertex(grip);
}

GripItem* MeshItem::activeVertex() const
{
  return m_lastVertex.data();
}

void MeshItem::setActiveVertex(GripItem* vertex)
{
  m_lastVertex = vertex;
  if (!vertex) {
    m_lastVertexFocus->hide();
    return;
  }
  m_lastVertexFocus->show();
  m_lastVertexFocus->setPos(vertex->pos());
}

GripItem* MeshItem::addVertexToPolygon(const QPointF& pos)
{
  if (!m_lastVertex) {
    // No place to connect the edge to
    return nullptr;
  }
  return nullptr;
}

bool MeshItem::splitPolygon(GripItem* v1, GripItem* v2)
{
  QSet<Polygon*> polys = polygonsContainingVertex(v1);
  polys.intersect(polygonsContainingVertex(v2));
  if (!polys.size()) {
    // The two vertices are in different polygons.
    return false;
  }
  QLineF newLine(v1->pos(), v2->pos());
  for (Polygon* poly : polys) {
    if (!poly->windingDirection) {
      // A degenerate polygon cannot be split
      continue;
    }
    QSet<EdgeItem*> edges1 = poly->edgesContainingVertex(v1);
    QSet<EdgeItem*> edges2 = poly->edgesContainingVertex(v2);
    if (edges1.intersects(edges2)) {
      // v1 and v2 already have a shared edge
      return false;
    }
    if (edges1.size() != 2) {
      // This shouldn't be possible, but as a sanity check...
      continue;
    }
    EdgeItem* edge1 = *edges1.begin();
    EdgeItem* edge2 = *(edges1.begin() + 1);
    GripItem* vertexBefore = edge1->leftGrip() == v1 ? edge1->rightGrip() : edge1->leftGrip();
    GripItem* vertexAfter = edge2->leftGrip() == v1 ? edge2->rightGrip() : edge2->leftGrip();
    if (poly->vertices.indexOf(vertexBefore) > poly->vertices.indexOf(vertexAfter)) {
      std::swap(edge1, edge2);
      std::swap(vertexBefore, vertexAfter);
    }
    QLineF e1 = edge1->line();
    QLineF e2 = edge2->line();
    double vertexAngle = signedAngle(vertexBefore->pos(), v1->pos(), vertexAfter->pos());
    double a1 = signedAngle(vertexBefore->pos(), v1->pos(), v2->pos());
    if (vertexAngle < 0) {
      vertexAngle = -vertexAngle;
      a1 = -a1;
    }
    if (0 < a1 && a1 < vertexAngle) {
      qDebug() << "can split";
      return true;
    }
  }
  // The edge that would be created is not in the interior
  // of any of the polygons that we found.
  return false;
}

void MeshItem::gripDestroyed(QObject* grip)
{
  if (grip == m_lastVertex) {
    m_lastVertexFocus->hide();
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

    if (!poly.windingDirection) {
      poly.updateWindingDirection();
    }
    program->setUniformValue("windingDirection", poly.windingDirection);

    gl->glDrawArrays(GL_TRIANGLE_FAN, 0, vbo.count());
  }

  painter->endNativePainting();
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

QSet<EdgeItem*> MeshItem::Polygon::edgesContainingVertex(GripItem* vertex)
{
  QSet<EdgeItem*> result;

  for (EdgeItem* edge : edges) {
    if (edge->hasGrip(vertex)) {
      result += edge;
    }
  }

  return result;
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
    double angle = signedAngle(a, b, c);
    if (angle > 0) {
      windingDirection += 1.0f;
    } else if (angle < 0) {
      windingDirection -= 1.0f;
    }
    a = b;
    b = c;
  }
  windingDirection = std::signbit(windingDirection) ? -1.0f : 1.0f;
}
