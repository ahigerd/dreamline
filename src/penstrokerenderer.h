#ifndef DL_PENSTROKERENDERER_H
#define DL_PENSTROKERENDERER_H

#include "abstractmeshrenderer.h"
class PenStrokePropertyPanel;

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
};

#endif
