#version 330 core
#define N <N>

uniform vec2[N] verts;
uniform vec4[N] colors;
in vec2 pt;

out vec4 FragColor;

float coord(vec2 a, vec2 b)
{
  return tan(atan((a.x * b.y) - (a.y * b.x), dot(a, b)) * 0.5);
}

void main()
{
  FragColor = vec4(0, 0, 0, 0);

  vec2 prev = verts[N - 3];
  vec2 curr = verts[N - 2];
  vec2 next = verts[N - 1];
  vec2 normPt;

  float w;
  float t = 0;

  for (int i = 0; i < N; i++) {
    normPt = normalize(curr - pt);
    w = (
      coord(prev - pt, normPt) +
      coord(normPt, next - pt)
    ) / distance(curr, pt);

    FragColor += colors[(i + N - 2) % N] * w;
    t += w;

    prev = curr;
    curr = next;
    next = verts[i];
  }

  FragColor /= t;
}
