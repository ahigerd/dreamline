#version 330 core
#define N <N>

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 control1;
layout (location = 2) in vec2 control2;
layout (location = 3) in vec2 control3;
uniform vec2[N] verts;
uniform vec2 translate;
uniform vec2 scale;

out vec2 pt;
flat out vec2[3] control;

void main()
{
  gl_Position = vec4(pos * scale + translate, 0.0f, 1.0f);
  pt = pos;
  control[0] = control1;
  control[1] = control2;
  control[2] = control3;
}
