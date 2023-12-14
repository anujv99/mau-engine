#version 460 core

layout (location = 0) out vec4 out_color;

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec2 tex_coord;

layout (set = 1, binding = 0) uniform sampler2D tex_sampler[];

void main() {
  vec3 tex_color = texture(tex_sampler[0], tex_coord).rgb;

  out_color = vec4(tex_color, 1.0);
}
