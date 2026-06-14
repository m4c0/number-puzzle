#version 450

layout(push_constant) uniform upc {
  float aspect;
} pc;

layout(binding = 0) uniform sampler2D txt;

layout(location = 0) in vec2 f_pos;

layout(location = 0) out vec4 colour;

void main() {
  vec4 c = texture(txt, f_pos * 0.5 + 0.5);
  colour = c;
}
