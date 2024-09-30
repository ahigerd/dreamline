#version 330 core
#define N <N>
#define PI 3.141592653589793
#define PI2 6.283185307179586

layout (location = 0) in vec2 pos;
uniform vec2[N] verts;
uniform vec2 translate;
uniform vec2 scale;

out float windingDirection;
out vec2 pt;

void main()
{
  gl_Position = vec4(pos * scale + translate, 0.0f, 1.0f);
  pt = pos;
  windingDirection = 0;
  for (int i = 0; i < N; i++) {
    vec2 a = verts[i] - verts[(i + 1) % N];
    vec2 b = verts[(i + 2) % N] - verts[(i + 1) % N];
    float angle = mod(atan(a.y, a.x) - atan(b.y, b.x) + PI, PI2) - PI;
    windingDirection += sign(angle);
  }
  windingDirection = sign(windingDirection + 0.5);
}
