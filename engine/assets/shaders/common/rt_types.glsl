
struct HitMaterial {
  vec3 albedo;
};

struct RayPayload {
  vec3        ray_dir;
  HitMaterial material;
  vec3        normal;
  float       distance;
};
