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

vec4 c_main(vec2 p, float sc) {
  float d = sd_box(p, vec2(sc));

  vec4 c = texture(txt, (p / sc) * 0.5 + 0.5);
  c = mix(c, vec4(0.2, 0.1, 0.05, 1.0), step(0, d));
  return c;
}

void main() {
  colour = c_main(f_pos, 0.9);
}
