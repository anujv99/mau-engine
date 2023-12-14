// vim: set ft=glsl:

#version 460
#extension GL_EXT_ray_tracing : require

#include "common/rt_types.glsl"

layout(location = 0) rayPayloadInEXT RayPayload ray_payload;

void main() {
  const vec3 direction = ray_payload.ray_dir;
  const float a = 0.5f * (direction.y + 1.0f);
  const vec3 color = mix(vec3(1.0f), vec3(0.5f, 0.7f, 1.0f), a);

  ray_payload.material.albedo = color;
  ray_payload.normal          = vec3(0.0);
  ray_payload.distance        = -1.0;
}
