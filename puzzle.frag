#version 450

layout(push_constant) uniform upc {
  float aspect;
} pc;

layout(binding = 0) uniform sampler2D txt;

void main() {
}
