#version 450

layout(push_constant) uniform upc {
  float aspect;
} pc;

void main() {
  vec2 pos = vec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1) * 3;
  vec2 p = pos * 2.0 - 1.0;
  gl_Position = vec4(p, 0, 1);
}

