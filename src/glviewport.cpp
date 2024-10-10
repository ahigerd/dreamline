#include "glviewport.h"
#include "editorview.h"
#include <QWindow>
#include <QGraphicsView>
#include <QGraphicsItem>

GLViewport* GLViewport::instance(QOpenGLContext* ctx)
{
  return dynamic_cast<GLViewport*>(GLFunctions::instance(ctx));
}

GLViewport::GLViewport(QWidget* parent)
: QOpenGLWidget(parent), GLFunctions(this)
{
  // initializers only
}

GLViewport::~GLViewport()
{
  makeCurrent();
  QGraphicsView* view = qobject_cast<QGraphicsView*>(parentWidget());
  if (view) {
    qDeleteAll(view->items());
  }
}

QTransform GLViewport::transform() const
{
  QGraphicsView* view = qobject_cast<QGraphicsView*>(parentWidget());
  if (view) {
    QPointF tl = view->mapToScene(0, 0);
    QPointF br = view->mapToScene(width(), height());
    QPointF center = (tl + br) / 2.0;
    float scaleX = 2.0 / (br.x() - tl.x());
    float scaleY = 2.0 / (br.y() - tl.y());
    return QTransform(scaleX, 0, 0, -scaleY, center.x() * -scaleX, center.y() * scaleY);
  }
  return QTransform();
}

EditorView* GLViewport::editor() const
{
  return dynamic_cast<EditorView*>(parent());
}

void GLViewport::initializeGL()
{
  initialize(context());
}
