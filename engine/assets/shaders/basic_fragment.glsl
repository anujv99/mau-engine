#version 460
#pragma shader_stage(fragment)

#include "common/limits.glsl"

#extension GL_EXT_nonuniform_qualifier : require

struct GPUMaterial {
  uint Diffuse;
  uint Normal;
};

layout (location = 0) out vec4 out_color;

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec2 tex_coord;
layout (location = 2) flat in uint material_index;

layout (set = 1, binding = 0) uniform sampler2D tex_sampler[];
layout (set = 2, binding = 0, std140) uniform UniformBuffer {
  GPUMaterial materials[MAX_MODELS];
} material_buffer;

void main() {
  GPUMaterial material = material_buffer.materials[nonuniformEXT(material_index)];
  vec3 tex_color = texture(tex_sampler[nonuniformEXT(material.Diffuse)], tex_coord).rgb;

  out_color = vec4(tex_color, 1.0);
}
