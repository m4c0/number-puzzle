#version 450

layout(push_constant) uniform upc {
  float aspect;
} pc;

layout(binding = 0) uniform sampler2D txt;

layout(location = 0) in vec2 f_pos;

layout(location = 0) out vec4 colour;

const uint w = 5;

float sd_box(vec2 p, vec2 b) {
  vec2 d = abs(p) - b;
  return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
} 

float sd_main_box(vec2 p, float sc) {
  const float b = 0.1;
  float d = sd_box(p, vec2(sc - b)) - b;
  return step(0, d);
}

void main() {
  const float sc = 0.9;
  vec2 p = f_pos / sc;

  vec2 uv = p * 0.5 + 0.5;
  vec3 t = texture(txt, uv).rgb;

  float lim = 1 - step(0, sd_box(p, vec2(1)));

  p = p * 0.5 + 0.5;
  p = fract(p);
  p = p * float(w);
  uvec2 id = uvec2(p);
  p = fract(p);
  p = p * 2 - 1;

  float d = 1 - sd_main_box(p, 0.95);
  d = d * lim;

  if (id == uvec2(w - 1)) d = 0;

  vec3 c = mix(vec3(0.2, 0.1, 0.05), t, d);
  colour = vec4(c, 1);
}
