#include "meshitem.h"
#include "meshrenderdata.h"
#include "glviewport.h"
#include "gripitem.h"
#include "edgeitem.h"
#include "polylineitem.h"
#include "editorview.h"
#include "mathutil.h"
#include "meshgradientrenderer.h"
#include "penstrokerenderer.h"
#include "propertypanel.h"
#include <QJsonArray>
#include <QOpenGLVertexArrayObject>
#include <QPainter>
#include <QPointer>
#include <QVector>
#include <limits>

class MeshItemPrivate : public MeshRenderData
{
public:
  MeshItemPrivate(MeshItem* p)
  : p(p), edgesVisible(true), verticesVisible(true), fill(new MeshGradientRenderer), stroke(nullptr)
  {
    // TODO: It might be a better idea to render this in paint() or
    // DreamProject::drawForeground() instead of using QGraphicsItems.
    lastVertexFocus = new QGraphicsEllipseItem(-8.5, -8.5, 17, 17, p);
    lastVertexFocus->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    QPen pen(Qt::black, 3.8);
    pen.setCosmetic(true);
    lastVertexFocus->setPen(pen);
    QGraphicsEllipseItem* inner = new QGraphicsEllipseItem(-8.5, -8.5, 17, 17, lastVertexFocus);
    pen.setColor(Qt::white);
    pen.setWidthF(1.4);
    inner->setPen(pen);
    lastVertexFocus->setZValue(100);
    lastVertexFocus->hide();
  }

  MeshItem* p;
  QVector<GripItem*> grips, boundary;
  QVector<EdgeItem*> edges;
  QPointer<GripItem> lastVertex;
  QGraphicsEllipseItem* lastVertexFocus;
  QPen strokePen;
  bool edgesVisible, verticesVisible;

  std::unique_ptr<IMeshRenderer> fill;
  std::unique_ptr<IMeshRenderer> stroke;
  QPointer<PropertyPanel> fillPanel;
  QPointer<PropertyPanel> strokePanel;

  QSet<MeshPolygon*> polygonsContainingVertex(GripItem* vertex);
  MeshPolygon* findSplittablePolygon(GripItem* v1, GripItem* v2);
  EdgeItem* findOrCreateEdge(GripItem* v1, GripItem* v2);
  void recomputeBoundaries();
};

MeshItem::MeshItem(QGraphicsItem* parent)
: QObject(nullptr), QGraphicsPolygonItem(parent), d(new MeshItemPrivate(this))
{
  setFlag(QGraphicsItem::ItemIsMovable, true);

  setStrokePen(QPen(Qt::black, 3));
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
    if (vertex.count() > 6) {
      grip->setSmooth(vertex[6].toBool());
    }
  }

  for (const QJsonValue& polygonV : source["polygons"].toArray()) {
    MeshPolygon polygon(this);
    for (const QJsonValue& indexV : polygonV.toArray()) {
      int index = indexV.toInt(-1);
      if (index < 0 || index > d->grips.length()) {
        // TODO: error handling
        continue;
      }
      if (!polygon.vertices.isEmpty()) {
        polygon.edges.append(d->findOrCreateEdge(polygon.vertices.last(), d->grips[index]));
      }
      polygon.vertices.append(d->grips[index]);
    }
    polygon.edges.append(d->findOrCreateEdge(polygon.vertices.first(), polygon.vertices.last()));
    polygon.rebuildBuffers();
    d->polygons.append(polygon);
  }

  if (source.contains("fill")) {
    QJsonObject fill = source["fill"].toObject();
    // TODO: handle types once we have more than one
    d->fill->deserialize(fill, this, d);
  }

  d->stroke.reset(nullptr);
  if (source.contains("stroke")) {
    QJsonObject stroke = source["stroke"].toObject();
    QString type = stroke["type"].toString();
    if (type == "pen") {
      d->stroke.reset(new PenStrokeRenderer());
      d->stroke->deserialize(stroke, this, d);
    }
  }

  d->recomputeBoundaries();
}

MeshItem::~MeshItem()
{
  delete d;
}

QJsonObject MeshItem::serialize() const
{
  QJsonObject o;

  QJsonArray vertices;
  for (const GripItem* grip : d->grips) {
    QColor color = grip->color();
    QJsonArray vertex({
      grip->pos().x(),
      grip->pos().y(),
      color.red(),
      color.green(),
      color.blue(),
      color.alpha(),
      grip->isSmooth(),
    });
    vertices.append(vertex);
  }
  o["vertices"] = vertices;

  QJsonArray polygons;
  for (const MeshPolygon& polygon : d->polygons) {
    QJsonArray polyData;
    for (GripItem* grip : polygon.vertices) {
      polyData.append(d->grips.indexOf(grip));
    }
    polygons.append(polyData);
  }
  o["polygons"] = polygons;

  if (d->stroke) {
    o["stroke"] = d->stroke->serialize(this, d);
  }

  if (d->fill) {
    o["fill"] = d->fill->serialize(this, d);
  }

  return o;
}

const MeshRenderData* MeshItem::renderData() const
{
  return d;
}

bool MeshItem::edgesVisible() const
{
  if (d->edgesVisible) {
    GLViewport* gl = GLViewport::instance(QOpenGLContext::currentContext());
    if (gl && gl->editor()->edgesVisible()) {
      return true;
    }
  }
  return false;
}

void MeshItem::setEdgesVisible(bool on)
{
  d->edgesVisible = on;
  update();
}

bool MeshItem::verticesVisible() const
{
  if (d->verticesVisible) {
    GLViewport* gl = GLViewport::instance(QOpenGLContext::currentContext());
    if (gl && gl->editor()->verticesVisible()) {
      return true;
    }
  }
  return false;
}

void MeshItem::setVerticesVisible(bool on)
{
  d->verticesVisible = on;
  update();
}

QPen MeshItem::strokePen() const
{
  return d->strokePen;
}

void MeshItem::setStrokePen(const QPen& pen)
{
  d->strokePen = pen;
  if (!dynamic_cast<PenStrokeRenderer*>(d->stroke.get())) {
    d->stroke.reset(new PenStrokeRenderer());
  }
}

void MeshItem::removeStroke()
{
  d->stroke.reset();
  update();
}

PropertyPanel* MeshItem::fillPropertyPanel()
{
  if (!d->fillPanel && d->fill) {
    EditorView* e = editor();
    if (e) {
      d->fillPanel = d->fill->propertyPanel(e);
    }
  }
  return d->fillPanel;
}

PropertyPanel* MeshItem::strokePropertyPanel()
{
  if (!d->strokePanel && d->stroke) {
    EditorView* e = editor();
    if (e) {
      d->strokePanel = d->stroke->propertyPanel(e);
    }
  }
  return d->strokePanel;
}

GripItem* MeshItem::newGrip()
{
  GripItem* grip = new GripItem(this);
  d->grips.append(grip);
  QObject::connect(grip, SIGNAL(moved(GripItem*, QPointF)), this, SLOT(moveVertex(GripItem*, QPointF)));
  QObject::connect(grip, SIGNAL(colorChanged(MarkerItem*, QColor)), this, SLOT(changeColor(MarkerItem*, QColor)));
  QObject::connect(grip, SIGNAL(smoothChanged(MarkerItem*, bool)), this, SLOT(updateBoundary()));
  QObject::connect(grip, SIGNAL(destroyed(QObject*)), this, SLOT(gripDestroyed(QObject*)));
  return grip;
}

QSet<MeshPolygon*> MeshItemPrivate::polygonsContainingVertex(GripItem* vertex)
{
  QSet<MeshPolygon*> result;

  for (MeshPolygon& poly : polygons) {
    if (poly.vertices.contains(vertex)) {
      result += &poly;
    }
  }

  return result;
}

void MeshItem::moveVertex(GripItem* vertex, const QPointF& pos)
{
  int boundaryIndex = d->boundary.indexOf(vertex);
  if (boundaryIndex >= 0) {
    updateBoundary();
  }

  for (MeshPolygon& poly : d->polygons) {
    int index = poly.vertices.indexOf(vertex);
    if (index >= 0) {
      poly.setVertex(index, pos);
      poly.updateWindingDirection();
    }
  }

  if (vertex == d->lastVertex) {
    d->lastVertexFocus->setPos(pos);
  }
  emit modified(true);
}

void MeshItem::changeColor(MarkerItem* vertex, const QColor& color)
{
  for (MeshPolygon& poly : d->polygons) {
    int index = poly.vertices.indexOf(static_cast<GripItem*>(vertex));
    if (index >= 0) {
      poly.colors[index] = QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
  }
  emit modified(true);
}

void MeshItem::insertVertex(EdgeItem* edge, const QPointF& pos)
{
  int oldIndex = d->edges.indexOf(edge);
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
  d->edges.append(newEdge);

  int numRefs = 0;
  for (MeshPolygon& poly : d->polygons) {
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
      GripItem* pp1 = d->boundary[i];
      GripItem* pp2 = d->boundary[(i + 1) % len];
      if ((pp1 == p1 && pp2 == p2) || (pp1 == p2 && pp2 == p1)) {
        p.insert(i + 1, pos);
        d->boundary.insert(i + 1, grip);
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

int MeshItem::numBoundaryVertices() const
{
  return d->boundary.count();
}

GripItem* MeshItem::boundaryVertex(int index) const
{
  if (index < 0 || index >= d->boundary.count()) {
    return nullptr;
  }
  return d->boundary[index];
}

GripItem* MeshItem::activeVertex() const
{
  return d->lastVertex.data();
}

void MeshItem::setActiveVertex(GripItem* vertex)
{
  d->lastVertex = vertex;
  if (!vertex) {
    d->lastVertexFocus->hide();
    return;
  }
  d->lastVertexFocus->show();
  d->lastVertexFocus->setPos(vertex->pos());
}

bool MeshItem::splitPolygon(GripItem* v1, GripItem* v2)
{
  MeshPolygon* oldPoly = d->findSplittablePolygon(v1, v2);
  if (!oldPoly) {
    return false;
  }

  // We do this first because we might swap the vertices later.
  setActiveVertex(v2);

  d->polygons.append(MeshPolygon(this));
  MeshPolygon* newPoly = &d->polygons.back();

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
  EdgeItem* edge = d->findOrCreateEdge(v1, v2);
  oldPoly->edges.append(edge);
  newPoly->edges.append(edge);

  // Update cached data.
  oldPoly->rebuildBuffers();
  newPoly->rebuildBuffers();

  emit modified(true);
  return true;
}

MeshPolygon* MeshItemPrivate::findSplittablePolygon(GripItem* v1, GripItem* v2)
{
  QSet<MeshPolygon*> polys = polygonsContainingVertex(v1);
  polys.intersect(polygonsContainingVertex(v2));
  if (!polys.size()) {
    // The two vertices are in different polygons.
    return nullptr;
  }
  for (MeshPolygon* poly : polys) {
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
      d->grips += other->d->grips;
      d->edges += other->d->edges;
      d->polygons += other->d->polygons;
    }

    for (GripItem* grip : d->grips) {
      grip->setParentItem(this);
    }

    for (EdgeItem* edge : d->edges) {
      edge->setParentItem(this);
    }

    qDeleteAll(mergeWith);
  }

  MeshPolygon newPolygon(this);
  int numVertices = poly->pointCount();
  GripItem* lastGrip = poly->grip(numVertices - 1);
  QList<GripItem*> splicePoints;
  for (int i = 0; i < numVertices; i++) {
    GripItem* grip = poly->grip(i);
    if (!d->grips.contains(grip)) {
      d->grips << grip;
    }
    newPolygon.vertices << grip;
    EdgeItem* edge = d->findOrCreateEdge(lastGrip, grip);
    newPolygon.edges << edge;
  }
  newPolygon.rebuildBuffers();
  d->polygons << newPolygon;

  d->recomputeBoundaries();
}

void MeshItem::gripDestroyed(QObject* grip)
{
  if (grip == d->lastVertex) {
    d->lastVertexFocus->hide();
  }
}

void MeshItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
  painter->beginNativePainting();

  GLFunctions* gl = GLFunctions::instance(QOpenGLContext::currentContext());
  if (!gl) {
    qFatal("no context");
    return;
  }

  gl->glEnable(GL_BLEND);
  if (d->fill) {
    d->fill->render(this, d, painter, gl);
  }
  if (d->stroke) {
    d->stroke->render(this, d, painter, gl);
  }
  painter->setPen(QPen(Qt::black, 0));
  /*
  QPolygonF bounds;
  for (GripItem* grip : d->boundary) {
    bounds << grip->pos();
  }
  QPolygonF drawBounds = expandPolygon(bounds, strokePen().widthF() / 2.0);
  painter->drawPolygon(drawBounds);
  */
  /*
  for (int i = 0; i < d->boundaryTris.count(); i += 3) {
    QPolygonF tri;
    tri << d->boundaryTris[i] << d->boundaryTris[i + 1] << d->boundaryTris[i + 2];
    painter->drawPolygon(tri);
  }
  for (int i = 0; i < d->boundaryTris.count(); i += 3) {
    painter->drawLine(QLineF(d->boundaryTris[i + 2], (d->boundaryTris[i + 1] + d->boundaryTris[i]) / 2));
  }
  */
  //painter->drawPolygon(polygon());

  painter->endNativePainting();
}

EdgeItem* MeshItemPrivate::findOrCreateEdge(GripItem* v1, GripItem* v2)
{
  for (EdgeItem* edge : edges) {
    if (edge->hasGrip(v1) && edge->hasGrip(v2)) {
      return edge;
    }
  }
  EdgeItem* edge = new EdgeItem(v1, v2);
  QObject::connect(edge, SIGNAL(insertVertex(EdgeItem*,QPointF)), p, SLOT(insertVertex(EdgeItem*,QPointF)));
  edges.append(edge);
  return edge;
}

void MeshItem::updateBoundary()
{
  d->strokeOwner = nullptr;
  if (d->boundary.length() < 3) {
    return;
  }
  int boundaryLength = d->boundary.length();
  d->rawBoundary = QPolygonF(boundaryLength);
  QPolygonF tris;
  tris.reserve(boundaryLength * 3);
  QVector<QPointF> control;
  control.reserve(boundaryLength * 9);
  QVector<int> exterior;
  exterior.reserve(boundaryLength);
  QPointF prev = d->boundary.last()->pos();
  QPointF lastMidpoint = (prev + d->boundary[boundaryLength - 2]->pos()) / 2;
  bool lastSmooth = d->boundary.last()->isSmooth();

  int b = 0;
  for (GripItem* grip : d->boundary) {
    d->rawBoundary[b++] = grip->pos();
  }
  QPolygonF boundary = d->rawBoundary;
  b = 0;
  for (GripItem* grip : d->boundary) {
    QPointF curr = grip->pos();
    QPointF midpoint = (curr + prev) / 2;

    if (lastSmooth) {
      QPointF barycenter = (prev + midpoint + lastMidpoint) / 3.0;
      bool isExterior = !d->rawBoundary.containsPoint(barycenter, Qt::WindingFill);
      if (isExterior) {
        int n = (b + boundaryLength - 1) % boundaryLength;
        boundary[n] = midpoint;
        boundary.insert(n, lastMidpoint);
      }
      for (int k = 0; k < 3; k++) {
        exterior << (isExterior ? 1 : 0);
        control << prev;
        control << lastMidpoint;
        control << midpoint;
      }
      tris << midpoint;
      tris << lastMidpoint;
      tris << prev;
    }

    prev = curr;
    lastMidpoint = midpoint;
    lastSmooth = grip->isSmooth();
    ++b;
  }

  d->boundaryTris = tris;
  d->exterior = exterior;
  d->controlPoints = control;
  setPolygon(boundary);

  for (MeshPolygon& poly : d->polygons) {
    poly.rebuildBuffers();
  }
}

void MeshItemPrivate::recomputeBoundaries()
{
  if (grips.length() < 3) {
    return;
  }

  // Construct an index mapping vertices to edges
  // TODO: Consider maintaining this instead of calculating it on demand?
  QMultiMap<GripItem*, EdgeItem*> edgeIndex;
  for (EdgeItem* edge : edges) {
    edgeIndex.insert(edge->leftGrip(), edge);
    edgeIndex.insert(edge->rightGrip(), edge);
  }

  // Start with the leftmost point on the polygon.
  // Break ties with the Y coordinate.
  // This point is guaranteed to be on the boundary.
  GripItem* start = nullptr;
  double minX = std::numeric_limits<double>::max();
  double minY = std::numeric_limits<double>::max();
  for (GripItem* vertex : grips) {
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
  boundary.clear();
  boundary << start;

  // Walk the edges of the bounding polygon using the "left hand on the wall" method
  while (!boundary.contains(lastVertex)) {
    boundary << lastVertex;
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

  p->updateBoundary();
}

DreamProject* MeshItem::project() const
{
  return dynamic_cast<DreamProject*>(scene());
}

EditorView* MeshItem::editor() const
{
  DreamProject* p = project();
  if (!p) {
    return nullptr;
  }
  return p->currentEditor();
}
