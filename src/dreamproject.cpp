#include "dreamproject.h"
#include "meshitem.h"
#include "glfunctions.h"
#include <QGraphicsRectItem>
#include <QPalette>
#include <QPainter>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

#define DPI 100

DreamProject::DreamProject(const QSizeF& pageSize, QObject* parent)
: QGraphicsScene(parent)
{
  setBackgroundBrush(QColor(139,134,128,255));

  setPageSize(pageSize);

  // TODO: A new project should remain blank, but this is convenient for testing.
  MeshItem* p = new MeshItem;
  addItem(p);
}

QSizeF DreamProject::pageSize() const
{
  return pageRect.size() / DPI;
}

void DreamProject::setPageSize(const QSizeF& size)
{
  double width = DPI * size.width();
  double height = DPI * size.height();
  pageRect = QRectF(-(width / 2), -(height / 2), width, height);
  setSceneRect(pageRect.adjusted(-DPI, -DPI, DPI, DPI));
}

void DreamProject::drawBackground(QPainter* p, const QRectF& rect)
{
  p->fillRect(rect, backgroundBrush());

  p->setBrush(Qt::white);
  QPen pen(Qt::black, 1);
  pen.setCosmetic(true);
  p->setPen(pen);
  p->drawRect(pageRect);
}

QImage DreamProject::render(int dpi)
{
  // TODO: Maybe this should be on a different thread

  QOffscreenSurface surface;
  QOpenGLContext ctx;
  ctx.setShareContext(QOpenGLContext::currentContext());
  ctx.create();
  surface.create();
  ctx.makeCurrent(&surface);

  QSizeF size = pageSize() * dpi;
  QOpenGLFramebufferObject fbo(size.toSize(), QOpenGLFramebufferObject::CombinedDepthStencil);
  fbo.bind();

  GLFunctions gl(&surface);
  gl.initialize(&ctx);

  // Map scene coordinates to output coordinates
  gl.glViewport(0, 0, size.width(), size.height());
  gl.setTransform(QTransform(2.0 / pageRect.width(), 0, 0, -2.0 / pageRect.height(), 0, 0));

  // TODO: make a renderable class maybe?
  for (MeshItem* mesh : itemsOfType<MeshItem>()) {
    mesh->renderGL();
  }

  return fbo.toImage();
}

bool DreamProject::exportToFile(const QString& path, const QByteArray& format, int dpi)
{
  QImage rendered = render(dpi);
  return rendered.save(path, format.constData());
}

void DreamProject::open(const QString& path)
{
  // TODO: remove this when new projects start off blank
  // TODO: it might be nice to have a command to load the contents of another
  //       file into this one without deleting anything (e.g. shape library)
  qDeleteAll(itemsOfType<MeshItem>());

  QFile f(path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    throw OpenException(tr("Unable to load %1 (error #%2)").arg(path).arg(int(f.error())));
  }

  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
  if (doc.isNull()) {
    throw OpenException(err.errorString());
  }

  QJsonObject pageSize = doc["page"].toObject();
  // If page size is not set, use a default
  setPageSize(QSizeF(pageSize["width"].toInt(8.5), pageSize["height"].toInt(11)));

  QJsonArray meshes = doc["meshes"].toArray();
  for (const QJsonValue& meshV : meshes) {
    // TODO: return warnings if invalid
    MeshItem* mesh = new MeshItem(meshV.toObject());
    QObject::connect(mesh, SIGNAL(modified(bool)), this, SIGNAL(projectModified(bool)));
    addItem(mesh);
  }
}

void DreamProject::save(const QString& path)
{
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    throw SaveException(tr("Unable to save %1 (error #%2)").arg(path).arg(int(f.error())));
  }

  QJsonObject o;

  QJsonObject sizeJson;
  sizeJson["width"] = pageSize().width();
  sizeJson["height"] = pageSize().height();
  o["page"] = sizeJson;

  QJsonArray meshes;
  for (MeshItem* mesh : itemsOfType<MeshItem>()) {
    meshes.append(mesh->serialize());
  }
  o["meshes"] = meshes;

  f.write(QJsonDocument(o).toJson(QJsonDocument::Compact));
}
