// Simple Kaleidoscope. (c) Inigo Quilez 2009.
vec4 FUNCNAME(vec2 tc)
{
    vec2 p = -1.0 + 2.0 * tc;
    float a = atan(p.y, p.x);
    float r = sqrt(dot(p, p));
    vec2 uv;
    uv.x = PREFIX(size) * a / 3.1416;
    uv.y = -PREFIX(time) + sin(PREFIX(size) * r + PREFIX(time)) + 0.7 * cos(PREFIX(time) + PREFIX(size) * a);
    float w = 0.5 + 0.5 * (sin(PREFIX(time) + PREFIX(size) * r) + 0.7 * cos(PREFIX(time) + PREFIX(size) * a));
    vec4 col = INPUT(mod(uv, 1.0));

    return vec4(col.rgb * w, col.a);
}
