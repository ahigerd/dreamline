#version 330 core
#define N <N>
#define PI 3.141592653589793
#define PI2 6.283185307179586

layout (location = 0) in vec2 pos;
uniform vec2[N] verts;
uniform vec2 translate;
uniform vec2 scale;

out float reverseWind;
out vec2 pt;

void main()
{
  gl_Position = vec4(pos * scale + translate, 0.0f, 1.0f);
  pt = pos;
  vec2 a = verts[0] - verts[1];
  vec2 b = verts[2] - verts[1];
  float angle = mod(atan(a.y, a.x) - atan(b.y, b.x) + PI, PI2) - PI;
  reverseWind = sign(angle);
}
