#ifndef DL_MESHGRADIENTRENDERER_H
#define DL_MESHGRADIENTRENDERER_H

#include "abstractmeshrenderer.h"

class MeshGradientRenderer : public AbstractMeshRenderer<BlankPropertyPanel>
{
public:
  MeshGradientRenderer();

  virtual void render(MeshItem* mesh, MeshRenderData* data, QPainter* painter, GLFunctions* gl);
  virtual QJsonObject serialize(const MeshItem* mesh, const MeshRenderData* data) const;
  virtual void deserialize(const QJsonObject& json, MeshItem* mesh, MeshRenderData* data) const;
};

#endif
