#ifndef DL_GLFUNCTIONS_H
#define DL_GLFUNCTIONS_H

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QMap>
#include <QString>
#include <QTransform>
#include "boundprogram.h"
class QOpenGLContext;
class QOpenGLWidget;

class GLFunctions : public QOpenGLFunctions
{
public:
  static GLFunctions* instance(QOpenGLContext* ctx);

  GLFunctions(QObject* surface);
  virtual ~GLFunctions();

  BoundProgram useShader(const QString& name, int n = 0);

  virtual QTransform transform() const;
  void setTransform(const QTransform& transform);

  void initialize(QOpenGLContext* ctx);

private:
  void activateGL();
  void addShader(QOpenGLShaderProgram* program, const QString& name, int n, QOpenGLShader::ShaderType type);

  QMap<QString, QOpenGLShaderProgram*> m_shaders;
  QOpenGLVertexArrayObject m_vao;
  QSurface* m_surface;
  QOpenGLWidget* m_widget;
  QOpenGLContext* m_ctx;
  QTransform m_transform;
};

#endif
