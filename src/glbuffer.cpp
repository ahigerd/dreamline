#include "glbuffer.h"

GLBufferBase::GLBufferBase(int glType, QOpenGLBuffer::Type type)
: QOpenGLBuffer(type), m_dirty(true), m_glType(glType)
{
  // initializers only
}

bool GLBufferBase::bind()
{
  if (!isCreated()) {
    create();
  }
  bool ok = QOpenGLBuffer::bind();
  if (ok && m_dirty) {
    build();
    m_dirty = false;
  }
  return ok;
}

int GLBufferBase::bufferSize() const
{
  return count() * elementSize();
}
