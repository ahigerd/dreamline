#ifndef DL_MESHRENDERDATA_H
#define DL_MESHRENDERDATA_H

#include "meshpolygon.h"
#include "glbuffer.h"
#include <QVariantMap>

// This class exists to provide a common virtual base class for plugin data
// to allow MeshRenderData to clean it up on destruction.
class ExtraRenderData
{
public:
  virtual ~ExtraRenderData() {}
};

class MeshRenderData
{
public:
  QList<MeshPolygon> polygons;
  GLBuffer<QPointF> boundaryTris, controlPoints;
  QPainterPath strokePath;
  double lastStrokeScale;
  void* strokeOwner = nullptr;
  std::unique_ptr<ExtraRenderData> fillExtra;
  std::unique_ptr<ExtraRenderData> strokeExtra;
};

#endif
