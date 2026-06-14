#version 450

layout(push_constant) uniform upc {
  float aspect;
} pc;

layout(binding = 0) uniform sampler2D txt;

layout(location = 0) in vec2 f_pos;

layout(location = 0) out vec4 colour;

float sd_box(vec2 p, vec2 b) {
  vec2 d = abs(p) - b;
  return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
} 

void main() {
  float d = sd_box(f_pos, vec2(1));
  d = step(0, d);
  vec4 c = texture(txt, f_pos * 0.5 + 0.5);
  colour = mix(c, vec4(0.2, 0.1, 0.05, 1.0), d);
}
