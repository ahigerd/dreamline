#ifndef DL_MESHRENDERDATA_H
#define DL_MESHRENDERDATA_H

#include "meshpolygon.h"
#include "glbuffer.h"

class MeshRenderData
{
public:
  QList<MeshPolygon> polygons;
  GLBuffer<QPointF> boundaryTris, controlPoints;
  QPainterPath strokePath;
  double lastStrokeScale;
  void* strokeOwner = nullptr;
};

#endif
