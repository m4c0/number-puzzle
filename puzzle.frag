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

float sd_digit(vec2 p, uint n) {
  // Digit data by P_Malin (https://www.shadertoy.com/view/4sf3RN)
  // Digit "4" changed to suit my style
  const int[] font = int[](0x75557, 0x22222, 0x74717, 0x74747, 0x55744, 0x71747, 0x71757, 0x74444, 0x75757, 0x75747);

  p = p / 0.8;
  float d = sd_box(p, vec2(1));

  p = p * 0.5 + 0.5;
  uvec2 u = uvec2(p * vec2(3, 5));
  uint bit = u.x + (4 - u.y) * 4;
  d = d * ((font[n] >> bit) & 1);

  return step(0, d);
}

vec3 c_number(vec2 p, vec3 c, uvec2 id) {
  p = p * vec2(2, 2.5);
  float d = sd_box(p, vec2(1));
  d = 1 - step(0, d);

  uint n = 1 + id.x + id.y * w;
  n = n / (p.x < 0 ? 10 : 1);
  n = n % 10;

  p.x = fract(p.x + 1);
  p.x = p.x * 2 - 1;
  d = d * (1 - sd_digit(p, n));

  return mix(c, vec3(1), d);
}

void main() {
  const float sc = 0.9;
  vec2 p = f_pos / sc;

  vec2 uv = p * 0.5 + 0.5;
  vec3 c = texture(txt, uv).rgb;

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

  c = c_number(p, c, id);
  c = mix(vec3(0.2, 0.1, 0.05), c, d);
  colour = vec4(c, 1);
}
