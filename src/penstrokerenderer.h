#ifndef DL_PENSTROKERENDERER_H
#define DL_PENSTROKERENDERER_H

#include "abstractmeshrenderer.h"

class PenStrokeRenderer : public AbstractMeshRenderer<BlankPropertyPanel>
{
public:
  PenStrokeRenderer();

  virtual void render(MeshItem* mesh, MeshRenderData* data, QPainter* painter, GLFunctions* gl);
};

#endif
