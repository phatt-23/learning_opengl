// Int impl

int when_eq(int x, int y) { return 1 - abs(sign(x - y)); }
int when_neq(int x, int y) { return abs(sign(x - y)); }
int when_gt(int x, int y) { return max(sign(x - y), 0); }
int when_lt(int x, int y) { return max(sign(y - x), 0); }
int when_ge(int x, int y) { return 1 - when_lt(x, y); }
int when_le(int x, int y) { return 1 - when_gt(x, y); }

// Float impl

float when_eq(float x, float y) { return 1.0 - abs(sign(x - y)); }
float when_neq(float x, float y) { return abs(sign(x - y)); }
float when_gt(float x, float y) { return max(sign(x - y), 0.0); }
float when_lt(float x, float y) { return max(sign(y - x), 0.0); }
float when_ge(float x, float y) { return 1.0 - when_lt(x, y); }
float when_le(float x, float y) { return 1.0 - when_gt(x, y); }

// Vec2 impl

vec2 when_eq(vec2 x, vec2 y) { return 1.0 - abs(sign(x - y)); }
vec2 when_neq(vec2 x, vec2 y) { return abs(sign(x - y)); }
vec2 when_gt(vec2 x, vec2 y) { return max(sign(x - y), 0.0); }
vec2 when_lt(vec2 x, vec2 y) { return max(sign(y - x), 0.0); }
vec2 when_ge(vec2 x, vec2 y) { return 1.0 - when_lt(x, y); }
vec2 when_le(vec2 x, vec2 y) { return 1.0 - when_gt(x, y); }

// Vec3 impl

vec3 when_eq(vec3 x, vec3 y) { return 1.0 - abs(sign(x - y)); }
vec3 when_neq(vec3 x, vec3 y) { return abs(sign(x - y)); }
vec3 when_gt(vec3 x, vec3 y) { return max(sign(x - y), 0.0); }
vec3 when_lt(vec3 x, vec3 y) { return max(sign(y - x), 0.0); }
vec3 when_ge(vec3 x, vec3 y) { return 1.0 - when_lt(x, y); }
vec3 when_le(vec3 x, vec3 y) { return 1.0 - when_gt(x, y); }

// Vec4 impl

vec4 when_eq(vec4 x, vec4 y) { return 1.0 - abs(sign(x - y)); }
vec4 when_neq(vec4 x, vec4 y) { return abs(sign(x - y)); }
vec4 when_gt(vec4 x, vec4 y) { return max(sign(x - y), 0.0); }
vec4 when_lt(vec4 x, vec4 y) { return max(sign(y - x), 0.0); }
vec4 when_ge(vec4 x, vec4 y) { return 1.0 - when_lt(x, y); }
vec4 when_le(vec4 x, vec4 y) { return 1.0 - when_gt(x, y); }
