#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;
uniform vec2 translate;
uniform vec2 scale;

out vec4 vertexColor;

void main()
{
  gl_Position = vec4(pos * scale + translate, 0.0f, 1.0f);
  vertexColor = color;
}
