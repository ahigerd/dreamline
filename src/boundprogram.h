#ifndef DL_BOUNDPROGRAM_H
#define DL_BOUNDPROGRAM_H

#include <QList>
#include <QOpenGLShaderProgram>
#include "glbuffer.h"
class QOpenGLVertexArrayObject;
class QOpenGLBuffer;
class GLFunctions;

class BoundProgram
{
public:
  BoundProgram(const BoundProgram& other) = delete;
  BoundProgram(BoundProgram&& other) = delete;
  ~BoundProgram();

  inline QOpenGLShaderProgram* operator->() { return program; }

  bool bindAttributeBuffer(int location, GLBufferBase& buffer, int offset = 0, int stride = -1);
  bool bindAttributeBuffer(const char* location, GLBufferBase& buffer, int offset = 0, int stride = -1);

  template <typename T>
  void setUniformValueArray(const char* location, GLBuffer<T>& buffer) {
    program->setUniformValueArray(location, reinterpret_cast<const GLfloat*>(buffer.vector().constData()), buffer.size(), buffer.elementLength());
  }

  GLFunctions* gl;
  QOpenGLShaderProgram* program;
  QOpenGLVertexArrayObject* vao;

private:
  friend class GLFunctions;
  BoundProgram(GLFunctions* gl, QOpenGLShaderProgram* program, QOpenGLVertexArrayObject* vao);

  QList<GLBufferBase*> boundBuffers;
};

#endif
