#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

layout (location = 0) out vec4 color;
layout (location = 1) out vec2 out_tex_coord;

layout (push_constant) uniform Constants {
  vec4 color;
  mat4 mvp;
} push_constant;

layout (set = 0, binding = 0) uniform UniformBuffer {
  mat4 model[];
} ubo;

void main() {
  gl_Position = push_constant.mvp * ubo.model[0] * vec4(pos, 1.0);
  color = vec4(tex_coord, 0.0, 1.0);
  out_tex_coord = tex_coord;
}
