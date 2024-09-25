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

GLViewport* GLViewport::instance(QOpenGLContext* ctx)
{
  return ctxMap.value(ctx);
}

GLViewport::GLViewport(QWidget* parent)
: QOpenGLWidget(parent)
{
  setAttribute(Qt::WA_NativeWindow);

  QSurfaceFormat format;
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setVersion(4, 1);

  windowHandle()->setSurfaceType(QSurface::OpenGLSurface);
  windowHandle()->setFormat(format);
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

BoundProgram GLViewport::useShader(const QString& name, int n)
{
  QString templatedName = name;
  if (n) {
    templatedName = QStringLiteral("%1_%2").arg(name).arg(n);
  }
  QOpenGLShaderProgram* program = m_shaders.value(templatedName);
  if (!program) {
    program = new QOpenGLShaderProgram(this);
    m_shaders[name] = program;
    addShader(program, name, n, QOpenGLShader::Fragment);
    addShader(program, name, n, QOpenGLShader::Vertex);
    bool ok = program->link();
    if (!ok) {
      qFatal(qPrintable(QStringLiteral("Shader linking failed in %1:\n%2").arg(templatedName).arg(program->log())));
    }
  }
  return BoundProgram(this, program, &m_vao);
}

void GLViewport::addShader(QOpenGLShaderProgram* program, const QString& name, int n, QOpenGLShader::ShaderType type)
{
  QString filename = QStringLiteral(":/shaders/%1.%2.glsl").arg(name).arg(shaderTypeNames.value(type));
  QFile file(filename);
  bool ok = false;
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString source = QString::fromUtf8(file.readAll());
    if (n) {
      source = source.replace("<N>", QString::number(n));
    }
    ok = program->addShaderFromSourceCode(type, source);
  }
  if (!ok) {
    qFatal(qPrintable(QStringLiteral("Shader compilation failed in %1:\n%2").arg(filename).arg(program->log())));
  }
}
