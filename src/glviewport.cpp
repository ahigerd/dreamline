#include "glviewport.h"
#include <QMap>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QPointer>
#include <QFile>
#include <QOpenGLBuffer>
#include <QWindow>
#include <QtDebug>
#include <QGraphicsView>
#include <QGraphicsItem>

static QMap<QOpenGLContext*, GLViewport*> ctxMap;
static const QMap<QOpenGLShader::ShaderType, QString> shaderTypeNames{
  { QOpenGLShader::Fragment, "fragment" },
  { QOpenGLShader::Vertex, "vertex" },
};

BoundProgram::BoundProgram(QOpenGLShaderProgram* program, QOpenGLVertexArrayObject* vao)
: program(program), vao(vao)
{
  program->bind();
  vao->bind();
}

BoundProgram::~BoundProgram()
{
  vao->release();
  program->release();
}

GLViewport* GLViewport::instance(QOpenGLContext* ctx)
{
  return ctxMap.value(ctx);
}

GLViewport::GLViewport(QWidget* parent)
: QOpenGLWidget(parent)
{
  // initializers only
  setAttribute(Qt::WA_NativeWindow);

  QSurfaceFormat format;
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setVersion(3,3);

  windowHandle()->setSurfaceType(QSurface::OpenGLSurface);
  windowHandle()->create();
}

GLViewport::~GLViewport()
{
  makeCurrent();
  m_vao.destroy();
  QGraphicsView* view = qobject_cast<QGraphicsView*>(parentWidget());
  if (view) {
    qDeleteAll(view->items());
  }
  ctxMap.remove(context());
}

void GLViewport::initializeGL()
{
  ctxMap[context()] = this;
  initializeOpenGLFunctions();

  m_vao.create();
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

BoundProgram GLViewport::useShader(const QString& name)
{
  QOpenGLShaderProgram* program = m_shaders.value(name);
  if (!program) {
    program = new QOpenGLShaderProgram(this);
    m_shaders[name] = program;
    addShader(program, name, QOpenGLShader::Fragment);
    addShader(program, name, QOpenGLShader::Vertex);
    bool ok = program->link();
    if (!ok) {
      qWarning(qPrintable(QStringLiteral("Error linking '%1'").arg(name)));
    }
  }
  return BoundProgram(program, &m_vao);
}

void GLViewport::addShader(QOpenGLShaderProgram* program, const QString& name, QOpenGLShader::ShaderType type)
{
  QString filename = QStringLiteral(":/shaders/%1.%2.glsl").arg(name).arg(shaderTypeNames.value(type));
  if (QFile::exists(filename)) {
    bool ok = program->addShaderFromSourceFile(type, filename);
    if (!ok) {
      qFatal(qPrintable(QStringLiteral("Shader compilation failed in %1:\n%2").arg(filename).arg(program->log())));
    }
  }
}
