#version 460
#extension GL_EXT_ray_tracing : require

#include "common/rt_types.glsl"

layout(location = 0) rayPayloadInEXT RayPayload ray_payload;

void main() {
  ray_payload.color = vec3(0.0);
  ray_payload.normal = vec3(0.0);
  ray_payload.distance = -1.0;
}