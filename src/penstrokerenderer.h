#ifndef DL_PENSTROKERENDERER_H
#define DL_PENSTROKERENDERER_H

#include "abstractmeshrenderer.h"
class PenStrokePropertyPanel;
class QDoubleSpinBox;
class QComboBox;
class QFrame;

class PenStrokeRenderer : public AbstractMeshRenderer<PenStrokePropertyPanel>
{
public:
  PenStrokeRenderer();

  virtual void render(MeshItem* mesh, MeshRenderData* data, QPainter* painter, GLFunctions* gl);
};

class PenStrokePropertyPanel : public PropertyPanel
{
Q_OBJECT
public:
  PenStrokePropertyPanel();

  virtual void updateAllProperties();

  bool eventFilter(QObject* obj, QEvent* event);

private slots:
  void setPenWidth(double width);
  void setJoinStyle();
  void setCapStyle();
  void setDashStyle();

private:
  void setButtonColor(const QColor& c);

  QFrame* color;
  QDoubleSpinBox* penWidth;
  QComboBox* joinStyle;
  QComboBox* capStyle;
  QComboBox* dashStyle;
};

#endif
