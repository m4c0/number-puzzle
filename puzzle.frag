#version 450

layout(push_constant) uniform upc {
  vec2  aspect;
  float time;
  float won;
  uint  sel_id;
  uint  w;
} pc;

layout(binding = 0) uniform sampler2D txt;
layout(binding = 1) readonly buffer brd {
  uint board[];
};

layout(location = 0) in vec2 f_pos;

layout(location = 0) out vec4 colour;

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
float sd_number(vec2 p, uint n) {
  p = p * vec2(2, 2.5);
  float d = sd_box(p, vec2(1));
  d = 1 - step(0, d);

  n = n / (p.x < 0 ? 10 : 1);
  n = n % 10;

  p.x = fract(p.x + 1);
  p.x = p.x * 2 - 1;
  d = d * (1 - sd_digit(p, n));
  return d;
}

vec3 c_number(vec2 p, vec3 c, uint id) {
  float fgn = 0;
  for (int my = -2; my <= 2; my++) {
    for (int mx = -2; mx <= 2; mx++) {
      vec2 m = vec2(mx, my) * 0.005;
      float fnm = sd_number(p - m, id);

      float gm = mat3(
        36, 24, 6,
        24, 16, 4,
        6, 4, 1
      )[abs(my)][abs(mx)];
      
      fgn += fnm * gm;
    }
  }
  fgn /= 256;

  c = mix(c, vec3(0), smoothstep(0.0, 0.5, fgn));
  c = mix(c, vec3(1), smoothstep(0.5, 1.0, fgn));
  return c;
}

// Noise based on shaders created by Inigo Quilez
// https://www.shadertoy.com/view/lsf3WH
float hash(ivec2 p) {
  // 2D -> 1D
  int n = p.x*3 + p.y*113;
  // 1D hash by Hugo Elias
  n = (n << 13) ^ n;
  n = n * (n * n * 15731 + 789221) + 1376312589;
  return -1.0+2.0*float( n & 0x0fffffff)/float(0x0fffffff);
}
float noise(vec2 p) {
  ivec2 i = ivec2(floor(p));
  vec2 f = fract(p);
  vec2 u = f*f*(3.0-2.0*f);

  return mix(mix(hash(i + ivec2(0, 0)), 
                 hash(i + ivec2(1, 0)), u.x),
             mix(hash(i + ivec2(0, 1)), 
                 hash(i + ivec2(1, 1)), u.x), u.y);
}
vec3 back_noise(vec2 p) {
  float f2 = dot(p, p);
  float f = sqrt(f2);
  vec2 uv = p / f2;
  float n = noise(4. * uv + pc.time * 0.1);
  return mix(vec3(0.2, 0.3, 0.4), vec3(0.1, 0.2, 0.3), n);
}

vec3 back() {
  vec2 p = f_pos / 0.9;
  float d = sd_box(p, vec2(1)) - 0.05;

  float db = abs(d);
  db = smoothstep(0.05, 0.06, db);

  vec3 cin = texture(txt, p * 0.5 + 0.5).rgb * 0.5;
  cin = mix(vec3(0), cin, smoothstep(0.05, 1.2, pow(db, 2)));
  cin = mix(vec3(0.3, 0.2, 0.1), cin, smoothstep(0.05, 0.06, db));

  vec3 cout = back_noise(p);
  return mix(cin, cout, step(0, d));
}

vec2 pick(in vec2 p, out float lim) {
  p = p / 0.9;

  lim = 1 - step(0, sd_box(p, vec2(1)));

  p = p * 0.5 + 0.5;
  p = fract(p);
  p = p * float(pc.w);
  return p;
}

void main() {
  float lim = 0;
  vec2 p = pick(f_pos, lim);
  uvec2 id2 = uvec2(p);
  uint id = id2.x + id2.y * pc.w;
  uint n = board[id];

  p = fract(p);
  uvec2 uvi = uvec2((n - 1) % pc.w, (n - 1) / pc.w);
  vec2 uv = (uvi + p) / float(pc.w);
  vec3 c = texture(txt, uv).rgb;

  p = p * 2 - 1;

  float d = 1 - sd_main_box(p, 0.95);
  d = d * lim;

  if (id == pc.sel_id) d *= 0.7;
  if (n == 0) d = 0;

  c = c_number(p, c, n);
  c = mix(back(), c, d);
  colour = vec4(c, 1);
}
