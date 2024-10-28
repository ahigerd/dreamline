#ifndef DL_ABSTRACTFILL_H
#define DL_ABSTRACTFILL_H

class MeshItem;
class MeshRenderData;
class QPainter;
class GLFunctions;
class QWidget;

class AbstractMeshRenderer
{
public:
  virtual ~AbstractMeshRenderer();

  virtual void render(MeshItem* mesh, MeshRenderData* data, QPainter* painter, GLFunctions* gl) = 0;

  // TODO: virtual QWidget* propertyPanel() = 0;

protected:
  AbstractMeshRenderer();
};

#endif
