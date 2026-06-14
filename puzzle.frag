#version 450

layout(push_constant) uniform upc {
  float aspect;
} pc;

layout(binding = 0) uniform sampler2D txt;

layout(location = 0) out vec4 colour;

void main() {
  colour = vec4(0.2, 0.1, 0.05, 1.0);
}
