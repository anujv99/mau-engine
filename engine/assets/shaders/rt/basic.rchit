#version 460

#include "common/limits.glsl"
#include "common/rt_types.glsl"

#define DEBUG_COLOR vec3(1.0, 1.0, 0.0)

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) rayPayloadInEXT RayPayload ray_payload;

hitAttributeEXT vec2 attribs;

struct Vertex {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoord;
};

struct GPUMaterial {
  uint Diffuse;
  uint Normal;

  uint pad1;
  uint pad2;
};

layout (buffer_reference, scalar) buffer Vertices { Vertex v[]; };
layout (buffer_reference, scalar) buffer Indices  { uvec3  i[]; };

struct ObjectDesc {
  uint64_t VertexAddress;
  uint64_t IndexAddress;
  uvec4    MaterialIndex;
};

layout (set = 1, binding = 0) uniform sampler2D tex_sampler[];
layout (set = 2, binding = 0, std140) uniform UniformBuffer {
  GPUMaterial materials[MAX_MODELS];
} material_buffer;
layout (set = 5, binding = 0, std140) uniform ObjectDescUniformBuffer {
  ObjectDesc desc[MAX_MODELS];
} object_desc;

void main() {
  if (gl_InstanceCustomIndexEXT < 0 || gl_InstanceCustomIndexEXT >= MAX_MODELS) {
	ray_payload.color = DEBUG_COLOR;
	return;
  }

  ObjectDesc desc = object_desc.desc[gl_InstanceCustomIndexEXT];

  Vertices    vertices       = Vertices(desc.VertexAddress);
  Indices     indices        = Indices(desc.IndexAddress);
  uint        material_index = desc.MaterialIndex.x;
  GPUMaterial material       = material_buffer.materials[nonuniformEXT(material_index)];

  uvec3 ind = indices.i[gl_PrimitiveID];
  
  Vertex v0 = vertices.v[ind.x];
  Vertex v1 = vertices.v[ind.y];
  Vertex v2 = vertices.v[ind.z];
  
  const vec3 bary = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  const vec3 pos = v0.Position * bary.x + v1.Position * bary.y + v2.Position * bary.z;
  const vec3 world_pos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));

  const vec3 nrm = v0.Normal * bary.x + v1.Normal * bary.y + v2.Normal * bary.z;
  const vec3 world_nrm = normalize(vec3(nrm * gl_WorldToObjectEXT));

  const vec2 tex_coord = v0.TexCoord * bary.x + v1.TexCoord * bary.y + v2.TexCoord * bary.z;

  vec3 color = DEBUG_COLOR;

  if (material.Diffuse != UINT32_MAX) {
	color = texture(tex_sampler[nonuniformEXT(material.Diffuse)], tex_coord).rgb;
  }

  ray_payload.color    = color;
  ray_payload.normal   = world_nrm;
  ray_payload.distance = gl_RayTmaxEXT;
}
