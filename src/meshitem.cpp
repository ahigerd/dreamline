#include "meshitem.h"
#include "glviewport.h"
#include "gripitem.h"
#include "edgeitem.h"
#include <QJsonArray>
#include <QOpenGLVertexArrayObject>
#include <QPainter>

MeshItem::MeshItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsPolygonItem(parent), m_edgesVisible(true), m_verticesVisible(true)
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
  polygonData.vertexBuffer = { QVector2D(poly[0]), QVector2D(poly[1]), QVector2D(poly[2]) };
  polygonData.colors = {
    { 1.0, 0.0, 0.0, 1.0 },
    { 0.0, 1.0, 0.0, 1.0 },
    { 0.0, 0.0, 1.0, 1.0 },
  };

  Polygon polygonData2;
  polygonData2.vertexBuffer = { QVector2D(poly[2]), QVector2D(poly[0]), QVector2D(poly[3]) };
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
        grip->setPos(p.vertex(i));
        grip->setBrush(p.color(i));
      }
      p.vertices.append(grip);
    }
    for (int i = 0; i < numVertices; i++) {
      GripItem* left = p.vertices[i];
      GripItem* right = p.vertices[(i + 1) % numVertices];
      EdgeItem* edge = findOrCreateEdge(left, right);
      p.edges.append(edge);
    }
  }

  m_lastVertexFocus = new QGraphicsEllipseItem(-8.5, -8.5, 17, 17, this);
  QPen pen(Qt::black, 3.8);
  pen.setCosmetic(true);
  m_lastVertexFocus->setPen(pen);
  QGraphicsEllipseItem* inner = new QGraphicsEllipseItem(-8.5, -8.5, 17, 17, m_lastVertexFocus);
  pen.setColor(Qt::white);
  pen.setWidthF(1.4);
  inner->setPen(pen);
  m_lastVertexFocus->setZValue(100);
  m_lastVertexFocus->hide();

  for (Polygon& poly : m_polygons) {
    poly.rebuildBuffers();
  }
}

MeshItem::MeshItem(const QJsonObject& source, QGraphicsItem* parent)
: MeshItem(parent)
{
  m_boundary.clear();
  m_polygons.clear();
  qDeleteAll(m_edges);
  m_edges.clear();
  qDeleteAll(m_grips);
  m_grips.clear();
  m_lastVertex = nullptr;

  for (const QJsonValue& vertexV : source["vertices"].toArray()) {
    // TODO: error handling
    QJsonArray vertex = vertexV.toArray();
    GripItem* grip = new GripItem(this);
    grip->setPos(vertex[0].toDouble(), vertex[1].toDouble());
    grip->setColor(QColor(vertex[2].toInt(), vertex[3].toInt(), vertex[4].toInt(), vertex[5].toInt(255)));
    m_grips.append(grip);
  }

  for (const QJsonValue& polygonV : source["polygons"].toArray()) {
    Polygon polygon;
    for (const QJsonValue& indexV : polygonV.toArray()) {
      int index = indexV.toInt(-1);
      if (index < 0 || index > m_grips.length()) {
        // TODO: error handling
        continue;
      }
      if (!polygon.vertices.isEmpty()) {
        polygon.edges.append(findOrCreateEdge(polygon.vertices.last(), m_grips[index]));
      }
      polygon.vertices.append(m_grips[index]);
    }
    polygon.rebuildBuffers();
    m_polygons.append(polygon);
  }

  // TODO: autocompute boundary if missing? Or just throw?
  for (const QJsonValue& indexV : source["boundary"].toArray()) {
    int index = indexV.toInt(-1);
    if (index < 0 || index > m_grips.length()) {
      // TODO: error handling
      continue;
    }
    m_boundary.append(m_grips[index]);
  }
}

QJsonObject MeshItem::serialize() const
{
  QJsonObject o;

  QJsonArray vertices;
  for (const GripItem* grip : m_grips) {
    QColor color = grip->color();
    QJsonArray vertex({
      grip->pos().x(),
      grip->pos().y(),
      color.red(),
      color.green(),
      color.blue(),
      color.alpha(),
    });
    vertices.append(vertex);
  }
  o["vertices"] = vertices;

  QJsonArray polygons;
  for (const Polygon& polygon : m_polygons) {
    QJsonArray polyData;
    for (GripItem* grip : polygon.vertices) {
      polyData.append(m_grips.indexOf(grip));
    }
    polygons.append(polyData);
  }
  o["polygons"] = polygons;

  QJsonArray boundary;
  for (GripItem* grip : m_boundary) {
    boundary.append(m_grips.indexOf(grip));
  }
  o["boundary"] = boundary;

  return o;
}

bool MeshItem::edgesVisible() const
{
  return m_edgesVisible;
}

void MeshItem::setEdgesVisible(bool on)
{
  m_edgesVisible = on;
  update();
}

bool MeshItem::verticesVisible() const
{
  return m_verticesVisible;
}

void MeshItem::setVerticesVisible(bool on)
{
  m_verticesVisible = on;
  update();
}

GripItem* MeshItem::newGrip()
{
  GripItem* grip = new GripItem(this);
  m_grips.append(grip);
  QObject::connect(grip, SIGNAL(moved(GripItem*, QPointF)), this, SLOT(moveVertex(GripItem*, QPointF)));
  QObject::connect(grip, SIGNAL(colorChanged(MarkerItem*, QColor)), this, SLOT(changeColor(MarkerItem*, QColor)));
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
      poly.setVertex(index, pos);
      poly.updateWindingDirection();
    }
  }

  if (vertex == m_lastVertex) {
    m_lastVertexFocus->setPos(pos);
  }
  emit modified(true);
}

void MeshItem::changeColor(MarkerItem* vertex, const QColor& color)
{
  for (Polygon& poly : m_polygons) {
    int index = poly.vertices.indexOf(static_cast<GripItem*>(vertex));
    if (index >= 0) {
      poly.colors[index] = QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
  }
  emit modified(true);
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

  grip->setColor(edge->colorAt(pos));

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
  emit modified(true);
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
  Polygon* oldPoly = findSplittablePolygon(v1, v2);
  if (!oldPoly) {
    return false;
  }

  // We do this first because we might swap the vertices later.
  setActiveVertex(v2);

  m_polygons.append(Polygon());
  Polygon* newPoly = &m_polygons.back();

  // Get the bounds of the vertices that need to move.
  int oldPos1 = oldPoly->vertices.indexOf(v1);
  int oldPos2 = oldPoly->vertices.indexOf(v2);
  if (oldPos1 > oldPos2) {
    std::swap(oldPos1, oldPos2);
    std::swap(v1, v2);
  }
  // Make sure not to splice out the first split vertex.
  // The end doesn't need to move because iterator ranges exclude end.
  oldPos1++;

  // To preserve the winding order of the existing vertices, the new
  // polygon must trace the new edge in the opposite direction.
  newPoly->vertices.append(v2);
  newPoly->vertices.append(v1);
  newPoly->vertices += oldPoly->vertices.mid(oldPos1, oldPos2 - oldPos1);

  // Remove the vertices that were spliced into the new polygon.
  oldPoly->vertices.erase(oldPoly->vertices.begin() + oldPos1, oldPoly->vertices.begin() + oldPos2);

  // Move the split-off edges to the new polygon.
  for (EdgeItem* edge : oldPoly->edges) {
    if (!oldPoly->vertices.contains(edge->leftGrip()) || !oldPoly->vertices.contains(edge->rightGrip())) {
      newPoly->edges.append(edge);
    }
  }
  for (EdgeItem* edge : newPoly->edges) {
    oldPoly->edges.removeAll(edge);
  }

  // Create the new edge.
  EdgeItem* edge = findOrCreateEdge(v1, v2);
  oldPoly->edges.append(edge);
  newPoly->edges.append(edge);

  // Update cached data.
  oldPoly->rebuildBuffers();
  newPoly->rebuildBuffers();

  emit modified(true);
  return true;
}

MeshItem::Polygon* MeshItem::findSplittablePolygon(GripItem* v1, GripItem* v2)
{
  QSet<Polygon*> polys = polygonsContainingVertex(v1);
  polys.intersect(polygonsContainingVertex(v2));
  if (!polys.size()) {
    // The two vertices are in different polygons.
    return nullptr;
  }
  for (Polygon* poly : polys) {
    if (poly->isEdgeInside(v1, v2)) {
      return poly;
    }
  }
  // The edge that would be created is not in the interior
  // of any of the polygons that we found.
  return nullptr;
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
  gl->glDisable(GL_MULTISAMPLE);
  gl->glEnable(GL_DITHER);
  for (Polygon& poly : m_polygons) {
    auto& vbo = poly.vertexBuffer;
    BoundProgram program = gl->useShader("polyramp", vbo.count());

    program.bindAttributeBuffer(0, vbo);
    program.bindAttributeBuffer(1, poly.colors);

    program.setUniformValueArray("verts", vbo);
    program.setUniformValueArray("colors", poly.colors);

    QTransform transform = gl->transform();
    program->setUniformValue("translate", transform.dx() + x() * transform.m11(), transform.dy() + y() * transform.m22());
    program->setUniformValue("scale", transform.m11(), transform.m22());

    if (!poly.windingDirection) {
      poly.updateWindingDirection();
    }
    program->setUniformValue("windingDirection", poly.windingDirection);

    gl->glDrawArrays(GL_TRIANGLE_FAN, 0, vbo.count());
  }
  gl->glEnable(GL_MULTISAMPLE);

  painter->endNativePainting();
}

EdgeItem* MeshItem::findOrCreateEdge(GripItem* v1, GripItem* v2)
{
  for (EdgeItem* edge : m_edges) {
    if (edge->hasGrip(v1) && edge->hasGrip(v2)) {
      return edge;
    }
  }
  EdgeItem* edge = new EdgeItem(v1, v2);
  QObject::connect(edge, SIGNAL(insertVertex(EdgeItem*,QPointF)), this, SLOT(insertVertex(EdgeItem*,QPointF)));
  m_edges.append(edge);
  return edge;
}
