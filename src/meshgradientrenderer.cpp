#include "meshgradientrenderer.h"
#include "meshitem.h"
#include "meshpolygon.h"
#include "glfunctions.h"
#include <QPainter>

MeshGradientRenderer::MeshGradientRenderer()
: AbstractMeshRenderer()
{
  // initializers only
}

void MeshGradientRenderer::render(MeshItem* mesh, MeshRenderData* data, QPainter*, GLFunctions* gl)
{
  gl->glDisable(GL_MULTISAMPLE);
  gl->glEnable(GL_DITHER);
  if (data->boundaryTris.count()) {
    gl->glEnable(GL_STENCIL_TEST);
    gl->glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  }
  for (MeshPolygon& poly : data->polygons) {
    auto& vbo = poly.vertexBuffer;
    BoundProgram program = gl->useShader("polyramp", vbo.count());

    program->setUniformValueArray("verts", vbo.vector().constData(), vbo.vector().size());
    program->setUniformValueArray("colors", poly.colors.vector().constData(), poly.colors.vector().size());

    QTransform transform = gl->transform();
    program->setUniformValue("translate", transform.dx() + mesh->x() * transform.m11(), transform.dy() + mesh->y() * transform.m22());
    program->setUniformValue("scale", transform.m11(), transform.m22());

    if (!poly.windingDirection) {
      poly.updateWindingDirection();
    }
    program->setUniformValue("windingDirection", poly.windingDirection);

    gl->glClear(GL_STENCIL_BUFFER_BIT);

    if (data->boundaryTris.count()) {
      gl->glStencilFunc(GL_ALWAYS, 1, 0xFF);
      gl->glStencilMask(0xFF);
      program.bindAttributeBuffer(0, data->boundaryTris);
      int controlSize = data->controlPoints.elementSize();
      int controlStride = controlSize * 3;
      for (int i = 0; i < 3; i++) {
        program.bindAttributeBuffer(i + 1, data->controlPoints, i * controlSize, controlStride);
      }
      program->setUniformValue("useEllipse", true);
      gl->glDrawArrays(GL_TRIANGLES, 0, data->boundaryTris.count());

      gl->glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      gl->glStencilMask(0x00);
    }
    program.bindAttributeBuffer(0, vbo);
    program->setUniformValue("useEllipse", false);
    gl->glDrawArrays(GL_TRIANGLE_FAN, 0, vbo.count());
  }
  gl->glDisable(GL_STENCIL_TEST);
  gl->glEnable(GL_MULTISAMPLE);
}
