#version 330 core
#define N <N>

layout (location = 0) in vec2 pos;
uniform vec2[N] verts;
uniform vec2 translate;
uniform vec2 scale;

out vec2 pt;

void main()
{
  gl_Position = vec4(pos * scale + translate, 0.0f, 1.0f);
  pt = pos;
}
