#ifndef DL_GLVIEWPORT_H
#define DL_GLVIEWPORT_H

#include <QOpenGLWidget>
#include "glfunctions.h"
class EditorView;

class GLViewport : public QOpenGLWidget, public GLFunctions
{
Q_OBJECT
public:
  static GLViewport* instance(QOpenGLContext* ctx);

  GLViewport(QWidget* parent = nullptr);
  ~GLViewport();

  QTransform transform() const;
  EditorView* editor() const;

protected:
  void initializeGL();
};

#endif
