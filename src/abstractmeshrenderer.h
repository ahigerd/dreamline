#ifndef DL_ABSTRACTFILL_H
#define DL_ABSTRACTFILL_H

class MeshItem;
class MeshRenderData;
class QPainter;
class GLFunctions;

class AbstractMeshRenderer
{
public:
  virtual ~AbstractMeshRenderer();

  virtual void render(MeshItem* mesh, MeshRenderData* data, QPainter* painter, GLFunctions* gl) = 0;

protected:
  AbstractMeshRenderer();
};

#endif
