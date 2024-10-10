#include "boundprogram.h"
#include "glfunctions.h"
#include "glbuffer.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QtDebug>

BoundProgram::BoundProgram(GLFunctions* gl, QOpenGLShaderProgram* program, QOpenGLVertexArrayObject* vao)
: gl(gl), program(program), vao(vao)
{
  program->bind();
  vao->bind();
}

BoundProgram::~BoundProgram()
{
  for (GLBufferBase* buffer : boundBuffers) {
    buffer->release();
  }
  vao->release();
  program->release();
}

bool BoundProgram::bindAttributeBuffer(int location, GLBufferBase& buffer, int offset, int stride)
{
  if (!buffer.bind()) {
    qDebug() << "bind failure";
    return false;
  }
  boundBuffers.append(&buffer);
  program->enableAttributeArray(location);
  if (stride < 0) {
    stride = buffer.elementSize();
  }
  program->setAttributeBuffer(location, buffer.m_glType, offset, buffer.elementLength(), stride);
  return true;
}

bool BoundProgram::bindAttributeBuffer(const char* location, GLBufferBase& buffer, int offset, int stride)
{
  return bindAttributeBuffer(program->attributeLocation(location), buffer, offset, stride);
}
