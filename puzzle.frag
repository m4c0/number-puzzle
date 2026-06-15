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

// Adapted from:
// =====================================
// Star Nest by Pablo Roman Andrioli
// License: MIT
// https://www.shadertoy.com/view/XlfGRj
// =====================================
vec3 star_nest() {

#define iterations 17
#define formuparam 0.53

#define volsteps 20
#define stepsize 0.1

#define zoom   0.800
#define tile   0.850
#define speed  0.001 

#define brightness 0.0015
#define darkmatter 0.300
#define distfading 0.730
#define saturation 0.850

  //get coords and direction
  vec2 uv = f_pos;
  vec3 dir=vec3(uv*zoom,1.);
  float time=pc.time*speed+.25;

  //mouse rotation
  float a1=.5;
  float a2=.8;
  mat2 rot1=mat2(cos(a1),sin(a1),-sin(a1),cos(a1));
  mat2 rot2=mat2(cos(a2),sin(a2),-sin(a2),cos(a2));
  dir.xz*=rot1;
  dir.xy*=rot2;
  vec3 from=vec3(1.,.5,0.5);
  from+=vec3(time*2.,time,-2.);
  from.xz*=rot1;
  from.xy*=rot2;

  //volumetric rendering
  float s=0.1,fade=1.;
  vec3 v=vec3(0.);
  for (int r=0; r<volsteps; r++) {
    vec3 p=from+s*dir*.5;
    p = abs(vec3(tile)-mod(p,vec3(tile*2.))); // tiling fold
    float pa,a=pa=0.;
    for (int i=0; i<iterations; i++) { 
      p=abs(p)/dot(p,p)-formuparam; // the magic formula
      a+=abs(length(p)-pa); // absolute sum of average change
      pa=length(p);
    }
    float dm=max(0.,darkmatter-a*a*.001); //dark matter
    a*=a*a; // add contrast
    if (r>6) fade*=1.-dm; // dark matter, don't render near
                          //v+=vec3(dm,dm*.5,0.);
    v+=fade;
    v+=vec3(s,s*s,s*s*s*s)*a*brightness*fade; // coloring based on distance
    fade*=distfading; // distance fading
    s+=stepsize;
  }
  v=mix(vec3(length(v)),v,saturation); //color adjust
  return v*.005;	
}

vec3 back() {
  float d = sd_box(f_pos, vec2(0.9)) - 0.05;

  vec3 cin  = vec3(0.2, 0.1, 0.05);
  vec3 cout = star_nest();
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
