#ifndef DL_GLBUFFER_H
#define DL_GLBUFFER_H

#include <QOpenGLBuffer>
#include <QVector>
#include <QPolygonF>
#include <QColor>
#include <QtDebug>

namespace GLBufferContainer {
  template <typename U>
  struct Container : public QVector<U> {
    using Type = QVector<U>;
  };

  template <>
  struct Container<QPointF> : public QPolygonF {
    using Type = QPolygonF;
  };
}

template <typename T>
class GLBuffer : public QOpenGLBuffer
{
public:
  using VectorType = typename GLBufferContainer::Container<T>::Type;
  using iterator = typename VectorType::iterator;
  using const_iterator = typename VectorType::const_iterator;

  GLBuffer(const QVector<T>& data = QVector<T>(), QOpenGLBuffer::Type type = QOpenGLBuffer::VertexBuffer)
  : QOpenGLBuffer(type), m_data(data), m_dirty(true)
  {
    // initializers only
  }

  iterator begin() { return m_data.begin(); }
  iterator end() { return m_data.end(); }
  const_iterator begin() const { return m_data.begin(); }
  const_iterator end() const { return m_data.end(); }

  inline int count() const { return m_data.length(); }
  inline int bufferSize() const { return m_data.length() * elementSize(); }
  int elementSize() const;

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

  bool bind()
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

protected:
  void build();

private:
  VectorType m_data;
  bool m_dirty;
};

template <typename T>
inline int GLBuffer<T>::elementSize() const
{
  return sizeof(T);
}

template <>
inline int GLBuffer<QPointF>::elementSize() const
{
  return 2 * sizeof(GLfloat);
}

template <>
inline int GLBuffer<QColor>::elementSize() const
{
  return 4 * sizeof(GLfloat);
}

template <typename T>
inline void GLBuffer<T>::build()
{
  allocate(m_data.constData(), bufferSize());
}

template <>
inline void GLBuffer<QPointF>::build()
{
  int numVertices = m_data.length();
  QVector<GLfloat> vertices(3 * numVertices);
  for (int i = 0, j = 0; i < numVertices; i++) {
    const QPointF& point = m_data[i];
    vertices[j++] = point.x();
    vertices[j++] = point.y();
  }
  allocate(vertices.constData(), bufferSize());
}

template <>
inline void GLBuffer<QColor>::build()
{
  int numColors = m_data.length();
  QVector<GLfloat> colors(4 * numColors);
  for (int i = 0, j = 0; i < numColors; i++) {
    const QColor& c = m_data[i];
    colors[j++] = c.redF();
    colors[j++] = c.greenF();
    colors[j++] = c.blueF();
    colors[j++] = c.alphaF();
  }
  allocate(colors.constData(), bufferSize());
}

#endif
