#version 460 core

layout (location = 0) in vec2 pos;

layout (location = 0) out vec4 color;

layout (push_constant) uniform Constants {
  vec4 color;
} push_constant;

void main() {
  gl_Position = vec4(pos, 0.0, 1.0);
  color = push_constant.color;
}
