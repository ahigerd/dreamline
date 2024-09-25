#ifndef DL_BOUNDPROGRAM_H
#define DL_BOUNDPROGRAM_H

#include <QList>
class QOpenGLShaderProgram;
class QOpenGLVertexArrayObject;
class QOpenGLBuffer;
class GLViewport;
class GLBufferBase;

class BoundProgram
{
public:
  BoundProgram(const BoundProgram& other) = delete;
  BoundProgram(BoundProgram&& other) = delete;
  ~BoundProgram();

  inline QOpenGLShaderProgram* operator->() { return program; }

  bool bindAttributeBuffer(int location, GLBufferBase& buffer, int offset = 0, int stride = -1);
  bool bindAttributeBuffer(const char* location, GLBufferBase& buffer, int offset = 0, int stride = -1);

  GLViewport* gl;
  QOpenGLShaderProgram* program;
  QOpenGLVertexArrayObject* vao;

private:
  friend class GLViewport;
  BoundProgram(GLViewport* gl, QOpenGLShaderProgram* program, QOpenGLVertexArrayObject* vao);

  QList<GLBufferBase*> boundBuffers;
};

#endif
