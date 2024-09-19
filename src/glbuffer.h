#ifndef DL_GLBUFFER_H
#define DL_GLBUFFER_H

#include <QOpenGLBuffer>
#include <QVector>
#include <QColor>
#include <QtDebug>

template <typename T>
class GLBuffer : public QOpenGLBuffer
{
public:
  GLBuffer(const QVector<T>& data = QVector<T>(), QOpenGLBuffer::Type type = QOpenGLBuffer::VertexBuffer)
  : QOpenGLBuffer(type), m_data(data), m_dirty(true)
  {
    // initializers only
  }

  typename QVector<T>::iterator begin() { return m_data.begin(); }
  typename QVector<T>::iterator end() { return m_data.end(); }
  typename QVector<T>::const_iterator begin() const { return m_data.begin(); }
  typename QVector<T>::const_iterator end() const { return m_data.end(); }

  inline int count() const { return m_data.length(); }
  inline int bufferSize() const { return m_data.length() * sizeof(T); }

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
  QVector<T> m_data;
  bool m_dirty;
};

template <typename T>
inline void GLBuffer<T>::build()
{
  allocate(m_data.constData(), bufferSize());
}

template <>
inline void GLBuffer<QPointF>::build()
{
  int numVertices = m_data.length();
  QVector<float> vertices(3 * numVertices);
  for (int i = 0, j = 0; i < numVertices; i++) {
    const QPointF& point = m_data[i];
    vertices[j++] = point.x();
    vertices[j++] = point.y();
    vertices[j++] = 0;
  }
  allocate(vertices.constData(), vertices.length() * sizeof(float));
}

template <>
inline void GLBuffer<QColor>::build()
{
  int numColors = m_data.length();
  QVector<float> colors(4 * numColors);
  for (int i = 0, j = 0; i < numColors; i++) {
    const QColor& c = m_data[i];
    colors[j++] = c.redF();
    colors[j++] = c.greenF();
    colors[j++] = c.blueF();
    colors[j++] = c.alphaF();
  }
  allocate(colors.constData(), colors.length() * sizeof(float));
}

#endif
