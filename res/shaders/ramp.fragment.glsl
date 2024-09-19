#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

void main()
{
  FragColor = vec4(vertexColor.r, vertexColor.g, vertexColor.b, 1.0f); /* vec4(1.0f, 0.5f, 0.2f, 1.0f); */
}
