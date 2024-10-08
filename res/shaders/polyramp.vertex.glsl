#version 330 core
#define N <N>

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;
uniform vec2[N] verts;
uniform vec2 translate;
uniform vec2 scale;

out vec2 pt;
out vec4 edgeColor;

void main()
{
  gl_Position = vec4(pos * scale + translate, 0.0f, 1.0f);
  edgeColor = color;
  pt = pos;
}
