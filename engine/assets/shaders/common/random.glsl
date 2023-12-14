
float rand(inout uint state) {
  state = state * 747796407 + 2891336453;
  uint result = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
  result = (result >> 22) ^ result;
  return float(result) / 4294967295.0f;
}

float rand(inout uint state, in float min_val, in float max_val) {
  return min_val + (max_val - min_val) * rand(state);
}

vec3 rand_vec3(inout uint state) {
  return vec3(rand(state), rand(state), rand(state));
}

vec3 rand_vec3(inout uint state, in float min_val, in float max_val) {
  return vec3(rand(state, min_val, max_val), rand(state, min_val, max_val), rand(state, min_val, max_val));
}

// length always < 1.0f
vec3 rand_in_unit_sphere(inout uint state) {
  while (true) {
    vec3 p = rand_vec3(state, -1.0f, 1.0f);
    if (dot(p, p) < 1.0f) return p;
  }
}

// length always == 1.0f
vec3 rand_unit_vector(inout uint state) {
  vec3 p = rand_vec3(state, -1.0f, 1.0f);
  return normalize(p);
}

vec3 rand_in_hemisphere(inout uint state, in vec3 normal) {
  vec3 p = rand_unit_vector(state);
  if (dot(p, normal) > 0.0f) return p;
  else return -p;
}
