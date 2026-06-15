#version 450

layout(push_constant) uniform upc {
  vec2  aspect;
  float time;
  float won;
  uint  sel_id;
  uint  w;
} pc;

layout(location = 0) out vec2 f_pos;

void main() {
  vec2 pos = vec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1) * 3;
  vec2 p = pos * 2.0 - 1.0;
  gl_Position = vec4(p, 0, 1);
  f_pos = p * pc.aspect;
}

