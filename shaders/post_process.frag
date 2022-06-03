#version 450 core

layout(binding = 1) uniform highp sampler2D in_color;

layout(location = 0) in highp vec2 in_uv;

layout(location = 0) out highp vec4 out_color;

vec3 aces_fitted(vec3 color);

vec3 ToSRGB(vec3 v);

void main()
{
    vec3 col = texture(in_color, in_uv).rgb;

    col = aces_fitted(col);

    // framebuffer srgb
    // col = ToSRGB(col);

    out_color = vec4(col, 1.0);
}

const float gamma = 2.2;

float saturate(float v)
{
    return clamp(v, 0.0, 1.0);
}

vec3 saturate(vec3 v)
{
    return vec3(saturate(v.x), saturate(v.y), saturate(v.z));
}

vec3 mul(mat3 m, vec3 v)
{
    return m * v;
}

vec3 rrt_odt_fit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

mat3 mat3_from_rows(vec3 c0, vec3 c1, vec3 c2)
{
    mat3 m = mat3(c0, c1, c2);
    m = transpose(m);

    return m;
}

vec3 aces_fitted(vec3 color)
{
    mat3 ACES_INPUT_MAT = mat3_from_rows(
        vec3(0.59719, 0.35458, 0.04823),
        vec3(0.07600, 0.90834, 0.01566),
        vec3(0.02840, 0.13383, 0.83777));

    mat3 ACES_OUTPUT_MAT = mat3_from_rows(
        vec3(1.60475, -0.53108, -0.07367),
        vec3(-0.10208, 1.10813, -0.00605),
        vec3(-0.00327, -0.07276, 1.07602));

    color = mul(ACES_INPUT_MAT, color);

    // Apply RRT and ODT
    color = rrt_odt_fit(color);

    color = mul(ACES_OUTPUT_MAT, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

vec3 ToSRGB(vec3 v)
{
    return PowVec3(v, 1.0 / gamma);
}