#version 330 core
#define N <N>

#define EPSILON 1e-37

uniform vec2[N] verts;
uniform vec4[N] colors;
uniform float windingDirection;
uniform bool useEllipse;
in vec2 pt;
in vec4 edgeColor;
flat in vec2[3] control;

out vec4 FragColor;

float coord(vec2 a, vec2 b)
{
  return tan(atan((a.x * b.y) - (a.y * b.x), dot(a, b)) * 0.5);
}

vec4 getColor(vec2 point)
{
  vec4 result = vec4(0, 0, 0, 0);

  vec2 prev = verts[N - 3];
  vec2 curr = verts[N - 2];
  vec2 next = verts[N - 1];
  vec2 normPt;

  float w;
  float t = 0;

  for (int i = 0; i < N; i++) {
    normPt = normalize(curr - point);
    w = windingDirection * (
      coord(prev - point, normPt) +
      coord(normPt, next - point)
    ) / distance(curr, point);

    result += colors[(i + N - 2) % N] * max(w, 0);
    t += w;

    prev = curr;
    curr = next;
    next = verts[i];
  }

  // 1e-37: Numerical instability causes calculations to be
  // off by 1 ULP, which results in gaps between polygons.
  if (t > 0) {
    if (result.a / t > EPSILON) {
      result /= t;
      return result;
    } else {
      return vec4(0, 0, 0, -1);
    }
  } else if (t > -1e30) {
    return vec4(0, 0, 0, 0);
  } else {
    return vec4(0, 0, 0, -1);
  }
}

void main()
{
  if (useEllipse) {
    vec2 a = control[0] - control[1];
    vec2 b = control[0] - control[2];
    vec2 origin = control[0] - a - b;
    mat2 mtx = inverse(mat2(a, b));
    vec2 tp = mtx * (pt - origin);
    float theta = atan(tp.x, tp.y);
    float s = 1 - sin(theta);
    float c = 1 - cos(theta);
    vec2 e = control[0] - a*s - b*c;
    float d1 = distance(origin, pt);
    float d2 = distance(origin, e);
    float distAlpha = 1;
    if (d1 > d2 + 1) {
      FragColor = vec4(0, 0, 0, 0);
      return;
    } else if (d1 > d2) {
      distAlpha = 1 - (d1 - d2);
    }
    float stepX = 0.1;
    float stepY = 0.1;
    FragColor = vec4(0, 0, 0, 1);
    float t = 0;
    float alpha = 0;
    for (int x = -1; x <= 1; x++) {
      for (int y = -1; y <= 1; y++) {
        if (x != 0 && abs(x) == abs(y)) continue;
        vec4 sampleColor = getColor(vec2(pt.x + x * stepX, pt.y + y * stepY));
        if (sampleColor.a > EPSILON) {
          FragColor.rgb += sampleColor.rgb;
          alpha = max(alpha, sampleColor.a);
          t += 1;
        }
      }
    }
    if (t <= 0 || FragColor.a <= 0) {
      FragColor = vec4(0, 0, 0, 0);
    } else {
      FragColor.rgb /= t;
      FragColor.a = alpha * distAlpha;
    }
  } else {
    FragColor = getColor(pt);
    if (FragColor.a < 0) {
      FragColor = edgeColor;
    }
  }
}
