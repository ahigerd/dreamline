#version 330 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec3 color;
uniform vec2 translate;
uniform vec2 scale;

out vec3 vertexColor;

void main()
{
  gl_Position = vec4(pos.x * scale.x + translate.x, pos.y * scale.y + translate.y, 0.0f, 1.0f);
  vertexColor = color;
}
