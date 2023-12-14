#version 460 core

layout (location = 0) in vec3 pos;

layout (location = 0) out vec4 color;

layout (push_constant) uniform Constants {
  vec4 color;
} push_constant;

layout (binding = 0) uniform UniformBuffer {
  mat4 model;
} ubo;

void main() {
  gl_Position = ubo.model * vec4(pos, 1.0);
  color = push_constant.color;
}
