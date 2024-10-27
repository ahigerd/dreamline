#include "meshitem.h"
#include "glviewport.h"
#include "gripitem.h"
#include "edgeitem.h"
#include "polylineitem.h"
#include "editorview.h"
#include "mathutil.h"
#include <QJsonArray>
#include <QOpenGLVertexArrayObject>
#include <QPainter>
#include <limits>

MeshItem::MeshItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsPolygonItem(parent), m_edgesVisible(true), m_verticesVisible(true)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);

  // TODO: It might be a better idea to render this in paint() or
  // DreamProject::drawForeground() instead of using QGraphicsItems.
  m_lastVertexFocus = new QGraphicsEllipseItem(-8.5, -8.5, 17, 17, this);
  m_lastVertexFocus->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  QPen pen(Qt::black, 3.8);
  pen.setCosmetic(true);
  m_lastVertexFocus->setPen(pen);
  QGraphicsEllipseItem* inner = new QGraphicsEllipseItem(-8.5, -8.5, 17, 17, m_lastVertexFocus);
  pen.setColor(Qt::white);
  pen.setWidthF(1.4);
  inner->setPen(pen);
  m_lastVertexFocus->setZValue(100);
  m_lastVertexFocus->hide();
}

MeshItem::MeshItem(PolyLineItem* polyline, QGraphicsItem* parent)
: MeshItem(parent)
{
  addPolygon(polyline);
}

MeshItem::MeshItem(const QJsonObject& source, QGraphicsItem* parent)
: MeshItem(parent)
{
  for (const QJsonValue& vertexV : source["vertices"].toArray()) {
    // TODO: error handling
    QJsonArray vertex = vertexV.toArray();
    GripItem* grip = newGrip();
    grip->setPos(vertex[0].toDouble(), vertex[1].toDouble());
    grip->setColor(QColor(vertex[2].toInt(), vertex[3].toInt(), vertex[4].toInt(), vertex[5].toInt(255)));
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
    polygon.edges.append(findOrCreateEdge(polygon.vertices.first(), polygon.vertices.last()));
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
  if (m_edgesVisible) {
    GLViewport* gl = GLViewport::instance(QOpenGLContext::currentContext());
    if (gl && gl->editor()->edgesVisible()) {
      return true;
    }
  }
  return false;
}

void MeshItem::setEdgesVisible(bool on)
{
  m_edgesVisible = on;
  update();
}

bool MeshItem::verticesVisible() const
{
  if (m_verticesVisible) {
    GLViewport* gl = GLViewport::instance(QOpenGLContext::currentContext());
    if (gl && gl->editor()->verticesVisible()) {
      return true;
    }
  }
  return false;
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
  QObject::connect(grip, SIGNAL(smoothChanged(MarkerItem*, bool)), this, SLOT(updateBoundary()));
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
    updateBoundary();
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

void MeshItem::addPolygon(PolyLineItem* poly)
{
  QList<MeshItem*> mergeWith = poly->attachedMeshes();
  mergeWith.removeAll(this);

  if (!mergeWith.isEmpty()) {
    // Here, we assume that the other meshes are fully disjoint.
    // If it were possible that a vertex or edge were shared,
    // we would need to deduplicate them.
    for (MeshItem* other : mergeWith) {
      m_grips += other->m_grips;
      m_edges += other->m_edges;
      m_polygons += other->m_polygons;
    }

    for (GripItem* grip : m_grips) {
      grip->setParentItem(this);
    }

    for (EdgeItem* edge : m_edges) {
      edge->setParentItem(this);
    }

    qDeleteAll(mergeWith);
  }

  Polygon newPolygon;
  int numVertices = poly->pointCount();
  GripItem* lastGrip = poly->grip(numVertices - 1);
  QList<GripItem*> splicePoints;
  for (int i = 0; i < numVertices; i++) {
    GripItem* grip = poly->grip(i);
    if (!m_grips.contains(grip)) {
      m_grips << grip;
    }
    newPolygon.vertices << grip;
    EdgeItem* edge = findOrCreateEdge(lastGrip, grip);
    newPolygon.edges << edge;
  }
  newPolygon.rebuildBuffers();
  m_polygons << newPolygon;

  recomputeBoundaries();
}

void MeshItem::gripDestroyed(QObject* grip)
{
  if (grip == m_lastVertex) {
    m_lastVertexFocus->hide();
  }
}

void MeshItem::renderGL()
{
  GLFunctions* gl = GLFunctions::instance(QOpenGLContext::currentContext());
  if (!gl) {
    qFatal("no context");
    return;
  }

  gl->glEnable(GL_BLEND);
  gl->glDisable(GL_MULTISAMPLE);
  gl->glEnable(GL_DITHER);
  if (m_boundaryTris.count()) {
    gl->glEnable(GL_STENCIL_TEST);
    gl->glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  }
  for (Polygon& poly : m_polygons) {
    auto& vbo = poly.vertexBuffer;
    BoundProgram program = gl->useShader("polyramp", vbo.count());

    program->setUniformValueArray("verts", vbo.vector().constData(), vbo.vector().size());
    program->setUniformValueArray("colors", poly.colors.vector().constData(), poly.colors.vector().size());

    QTransform transform = gl->transform();
    program->setUniformValue("translate", transform.dx() + x() * transform.m11(), transform.dy() + y() * transform.m22());
    program->setUniformValue("scale", transform.m11(), transform.m22());

    if (!poly.windingDirection) {
      poly.updateWindingDirection();
    }
    program->setUniformValue("windingDirection", poly.windingDirection);

    gl->glClear(GL_STENCIL_BUFFER_BIT);

    if (m_boundaryTris.count()) {
      gl->glStencilFunc(GL_ALWAYS, 1, 0xFF);
      gl->glStencilMask(0xFF);
      program.bindAttributeBuffer(0, m_boundaryTris);
      int controlSize = m_control.elementSize();
      int controlStride = controlSize * 3;
      for (int i = 0; i < 3; i++) {
        program.bindAttributeBuffer(i + 1, m_control, i * controlSize, controlStride);
      }
      program->setUniformValue("useEllipse", true);
      gl->glDrawArrays(GL_TRIANGLES, 0, m_boundaryTris.count());

      gl->glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      gl->glStencilMask(0x00);
    }
    program.bindAttributeBuffer(0, vbo);
    program->setUniformValue("useEllipse", false);
    gl->glDrawArrays(GL_TRIANGLE_FAN, 0, vbo.count());
  }
  gl->glDisable(GL_STENCIL_TEST);
  gl->glEnable(GL_MULTISAMPLE);

}

void MeshItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  painter->beginNativePainting();

  renderGL();

  painter->endNativePainting();

  /*
  QPen stroke(QColor(0, 0, 0, 128), 0);
  stroke.setCosmetic(true);
  painter->setPen(stroke);
  QPointF prev = m_boundary.last()->pos();
  QPointF lastMidpoint = (prev + m_boundary[m_boundary.length() - 2]->pos()) / 2;
  for (GripItem* grip : m_boundary) {
    QPointF curr = grip->pos();
    QPointF midpoint = (curr + prev) / 2;

    painter->drawLine(midpoint, lastMidpoint);

    prev = curr;
    lastMidpoint = midpoint;
  }
  */
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

void MeshItem::updateBoundary()
{
  if (m_boundary.length() < 3) {
    return;
  }
  QPolygonF tris(m_boundary.length() * 3);
  QVector<QPointF> control(m_boundary.length() * 9);
  QVector<int> smooth(m_boundary.length() * 3);
  QPointF prev = m_boundary.last()->pos();
  QPointF lastMidpoint = (prev + m_boundary[m_boundary.length() - 2]->pos()) / 2;
  int i = 0;
  int j = 0;
  bool lastSmooth = m_boundary.last()->isSmooth();
  for (GripItem* grip : m_boundary) {
    QPointF curr = grip->pos();
    QPointF midpoint = (curr + prev) / 2;

    if (lastSmooth) {
      for (int k = 0; k < 3; k++) {
        smooth[i + k] = lastSmooth ? 1 : 0;
        control[j++] = prev;
        control[j++] = lastMidpoint;
        control[j++] = midpoint;
      }
      tris[i++] = prev;
      tris[i++] = midpoint;
      tris[i++] = lastMidpoint;
    }

    prev = curr;
    lastMidpoint = midpoint;
    lastSmooth = grip->isSmooth();
  }
  m_boundaryTris = tris;
  m_control = control;
  m_smooth = smooth;
}

void MeshItem::recomputeBoundaries()
{
  if (m_grips.length() < 3) {
    return;
  }

  // Construct an index mapping vertices to edges
  // TODO: Consider maintaining this instead of calculating it on demand?
  QMultiMap<GripItem*, EdgeItem*> edgeIndex;
  for (EdgeItem* edge : m_edges) {
    edgeIndex.insert(edge->leftGrip(), edge);
    edgeIndex.insert(edge->rightGrip(), edge);
  }

  // Start with the leftmost point on the polygon.
  // Break ties with the Y coordinate.
  // This point is guaranteed to be on the boundary.
  GripItem* start = nullptr;
  double minX = std::numeric_limits<double>::max();
  double minY = std::numeric_limits<double>::max();
  for (GripItem* vertex : m_grips) {
    QPointF p = vertex->scenePos();
    if (minX > p.x() || (minX == p.x() && minY > p.y())) {
      start = vertex;
      minX = p.x();
      minY = p.y();
    }
  }

  // Pick the connected edge with the lowest Y coordinate.
  // Break ties with the X coordinate.
  // Because we're starting from the highest leftmost vertex, this guarantees that
  // the chosen edge is on the boundary.
  EdgeItem* lastEdge = nullptr;
  minX = std::numeric_limits<double>::max();
  minY = std::numeric_limits<double>::max();
  for (EdgeItem* edge : edgeIndex.values(start)) {
    GripItem* other = edge->otherGrip(start);
    QPointF p = other->scenePos();
    if (minY > p.y() || (minY == p.y() && minX > p.x())) {
      lastEdge = edge;
      minX = p.x();
      minY = p.y();
    }
  }
  if (!lastEdge) {
    qFatal("Degenerate geometry in MeshItem");
  }

  GripItem* lastVertex = lastEdge->otherGrip(start);
  GripItem* prevVertex = start;
  m_boundary.clear();
  m_boundary << start;

  // Walk the edges of the bounding polygon using the "left hand on the wall" method
  while (!m_boundary.contains(lastVertex)) {
    m_boundary << lastVertex;
    EdgeItem* nextEdge = nullptr;
    double maxAngle = 7; // bigger than 2pi
    for (EdgeItem* edge : edgeIndex.values(lastVertex)) {
      if (edge == lastEdge) {
        continue;
      }
      GripItem* nextVertex = edge->otherGrip(lastVertex);
      double angle = ccwAngle(prevVertex->scenePos(), lastVertex->scenePos(), nextVertex->scenePos());
      if (angle < maxAngle) {
        nextEdge = edge;
        maxAngle = angle;
      }
    }
    if (!nextEdge) {
      qFatal("Degenerate geometry in MeshItem");
    }
    prevVertex = lastVertex;
    lastVertex = nextEdge->otherGrip(lastVertex);
    lastEdge = nextEdge;
  }

  updateBoundary();
}
