#ifndef DL_GLBUFFER_H
#define DL_GLBUFFER_H

#include <QOpenGLBuffer>
#include <QVector>
#include <QVector4D>
#include <QPolygonF>
#include <QColor>
class BoundProgram;

namespace GLBufferContainer {
  template <typename T> struct Element {};

#define MAP_TYPE(CppType, GlType) template <> struct Element<CppType> { enum { Type = GlType, Bytes = sizeof(CppType), Length = 1 }; }
#define MAP_TYPE_VEC(CppType, GlType, ElementType, Count) template <> struct Element<CppType> { enum { Type = GlType, Bytes = sizeof(ElementType) * Count, Length = Count }; }
  MAP_TYPE(GLbyte, GL_BYTE);
  MAP_TYPE(GLubyte, GL_UNSIGNED_BYTE);
  MAP_TYPE(GLshort, GL_SHORT);
  MAP_TYPE(GLushort, GL_UNSIGNED_SHORT);
  MAP_TYPE(GLint, GL_INT);
  MAP_TYPE(GLuint, GL_UNSIGNED_INT);
  MAP_TYPE(GLfloat, GL_FLOAT);
  MAP_TYPE(GLdouble, GL_DOUBLE);
  MAP_TYPE_VEC(QPointF, GL_FLOAT, GLfloat, 2);
  MAP_TYPE_VEC(QColor, GL_FLOAT, GLfloat, 4);
  MAP_TYPE_VEC(QVector4D, GL_FLOAT, GLfloat, 4);
#undef MAP_TYPE
#undef MAP_TYPE_VEC

  template <typename T>
  struct Container : public QVector<T> {
    using Type = QVector<T>;

    static void allocate(QOpenGLBuffer* buffer, const Type& data, int size)
    {
      buffer->allocate(data.constData(), size);
    }
  };

  template <>
  struct Container<QPointF> : public QPolygonF {
    using Type = QPolygonF;

    static void allocate(QOpenGLBuffer* buffer, const Type& data, int size)
    {
      int numVertices = data.length();
      QVector<GLfloat> vertices(2 * numVertices);
      for (int i = 0, j = 0; i < numVertices; i++) {
        const QPointF& point = data[i];
        vertices[j++] = point.x();
        vertices[j++] = point.y();
      }
      buffer->allocate(vertices.constData(), size);
    }
  };

  template <>
  struct Container<QColor> : public QVector<QColor> {
    using Type = QVector<QColor>;

    static void allocate(QOpenGLBuffer* buffer, const Type& data, int size)
    {
      int numColors = data.length();
      QVector<GLfloat> colors(4 * numColors);
      for (int i = 0, j = 0; i < numColors; i++) {
        const QColor& c = data[i];
        colors[j++] = c.redF();
        colors[j++] = c.greenF();
        colors[j++] = c.blueF();
        colors[j++] = c.alphaF();
      }
      buffer->allocate(colors.constData(), size);
    }
  };
}

class GLBufferBase : public QOpenGLBuffer
{
public:
  GLBufferBase(int glType, QOpenGLBuffer::Type type);

  virtual int count() const = 0;
  virtual int elementSize() const = 0;
  virtual int elementLength() const = 0;
  int bufferSize() const;

  bool bind();

protected:
  friend class BoundProgram;
  virtual void build() = 0;

  bool m_dirty;
  int m_glType;
};

template <typename T, int glType = GLBufferContainer::Element<T>::Type>
class GLBuffer : public GLBufferBase
{
public:
  using VectorType = typename GLBufferContainer::Container<T>::Type;
  using iterator = typename VectorType::iterator;
  using const_iterator = typename VectorType::const_iterator;

  GLBuffer(const QVector<T>& data = QVector<T>(), QOpenGLBuffer::Type type = QOpenGLBuffer::VertexBuffer)
  : GLBufferBase(glType, type), m_data(data)
  {
    // initializers only
  }

  iterator begin() { return m_data.begin(); }
  iterator end() { return m_data.end(); }
  const_iterator begin() const { return m_data.begin(); }
  const_iterator end() const { return m_data.end(); }

  int count() const override { return m_data.length(); }
  int elementSize() const override { return GLBufferContainer::Element<T>::Bytes; }
  int elementLength() const override { return GLBufferContainer::Element<T>::Length; }

  const VectorType& vector() const { return m_data; };

  void resize(int count) { m_data.resize(count); }

  T& operator[](int pos)
  {
    m_dirty = true;
    return m_data[pos];
  }

  const T& operator[](int pos) const
  {
    return m_data[pos];
  }

  GLBuffer<T>& operator=(const QVector<T>& other)
  {
    m_dirty = true;
    m_data = other;
    return *this;
  }

  GLBuffer<T>& operator=(std::initializer_list<T> args)
  {
    m_dirty = true;
    m_data = args;
    return *this;
  }

protected:
  void build() override
  {
    GLBufferContainer::Container<T>::allocate(this, m_data, bufferSize());
  }

private:
  VectorType m_data;
};

#endif
