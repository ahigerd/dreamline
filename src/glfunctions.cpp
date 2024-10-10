#include "glfunctions.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOffscreenSurface>
#include <QWindow>
#include <QFile>
#include <QtDebug>

static QMap<QOpenGLContext*, GLFunctions*> ctxMap;
static const QMap<QOpenGLShader::ShaderType, QString> shaderTypeNames{
  { QOpenGLShader::Fragment, "fragment" },
  { QOpenGLShader::Vertex, "vertex" },
};

GLFunctions* GLFunctions::instance(QOpenGLContext* ctx)
{
  return ctxMap.value(ctx);
}

GLFunctions::GLFunctions(QObject* surface)
: QOpenGLFunctions(), m_surface(nullptr), m_widget(nullptr), m_ctx(nullptr)
{
  QSurfaceFormat format;
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setVersion(4, 1);
  format.setSamples(16);

  if (QOpenGLWidget* widget = dynamic_cast<QOpenGLWidget*>(surface)) {
    widget->setFormat(format);
    m_widget = widget;
  } else if (QWindow* window = dynamic_cast<QWindow*>(surface)) {
    window->setFormat(format);
    m_surface = window;
  } else if (QOffscreenSurface* offscreen = dynamic_cast<QOffscreenSurface*>(surface)) {
    offscreen->setFormat(format);
    m_surface = offscreen;
  }
}

GLFunctions::~GLFunctions()
{
  activateGL();
  m_vao.destroy();
  ctxMap.remove(m_ctx);
  qDeleteAll(m_shaders);
}

void GLFunctions::activateGL()
{
  if (m_widget) {
    m_widget->makeCurrent();
  } else if (m_surface && m_ctx) {
    m_ctx->makeCurrent(m_surface);
  }
}

void GLFunctions::initialize(QOpenGLContext* ctx)
{
  m_ctx = ctx;
  ctxMap[m_ctx] = this;
  initializeOpenGLFunctions();

  m_vao.create();
}

QTransform GLFunctions::transform() const
{
  return m_transform;
}

void GLFunctions::setTransform(const QTransform& transform)
{
  m_transform = transform;
}

BoundProgram GLFunctions::useShader(const QString& name, int n)
{
  QString templatedName = name;
  if (n) {
    templatedName = QStringLiteral("%1_%2").arg(name).arg(n);
  }
  QOpenGLShaderProgram* program = m_shaders.value(templatedName);
  if (!program) {
    program = new QOpenGLShaderProgram();
    m_shaders[templatedName] = program;
    addShader(program, name, n, QOpenGLShader::Fragment);
    addShader(program, name, n, QOpenGLShader::Vertex);
    bool ok = program->link();
    if (!ok) {
      qFatal(qPrintable(QStringLiteral("Shader linking failed in %1:\n%2").arg(templatedName).arg(program->log())));
    }
  }
  return BoundProgram(this, program, &m_vao);
}

void GLFunctions::addShader(QOpenGLShaderProgram* program, const QString& name, int n, QOpenGLShader::ShaderType type)
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
