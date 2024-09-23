#ifndef DL_GLVIEWPORT_H
#define DL_GLVIEWPORT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMap>
#include <QString>
#include <QTransform>
class QOpenGLContext;
class QOpenGLShaderProgram;

struct BoundProgram
{
  BoundProgram(QOpenGLShaderProgram* program, QOpenGLVertexArrayObject* vao);
  BoundProgram(const BoundProgram& other) = delete;
  BoundProgram(BoundProgram&& other) = delete;
  ~BoundProgram();

  inline QOpenGLShaderProgram* operator->() { return program; }

  QOpenGLShaderProgram* program;
  QOpenGLVertexArrayObject* vao;
};

class GLViewport : public QOpenGLWidget, public QOpenGLFunctions
{
Q_OBJECT
public:
  static GLViewport* instance(QOpenGLContext* ctx);

  GLViewport(QWidget* parent = nullptr);
  ~GLViewport();

  BoundProgram useShader(const QString& name, int n = 0);
  QTransform transform() const;

protected:
  void initializeGL();

private:
  void addShader(QOpenGLShaderProgram* program, const QString& name, int n, QOpenGLShader::ShaderType type);

  QMap<QString, QOpenGLShaderProgram*> m_shaders;
  QOpenGLVertexArrayObject m_vao;
};

#endif
