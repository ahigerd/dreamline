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
#include "boundprogram.h"
class QOpenGLContext;
class EditorView;

class GLViewport : public QOpenGLWidget, public QOpenGLFunctions
{
Q_OBJECT
public:
  static GLViewport* instance(QOpenGLContext* ctx);

  GLViewport(QWidget* parent = nullptr);
  ~GLViewport();

  BoundProgram useShader(const QString& name, int n = 0);
  QTransform transform() const;
  EditorView* editor() const;

protected:
  void initializeGL();

private:
  void addShader(QOpenGLShaderProgram* program, const QString& name, int n, QOpenGLShader::ShaderType type);

  QMap<QString, QOpenGLShaderProgram*> m_shaders;
  QOpenGLVertexArrayObject m_vao;
};

#endif
