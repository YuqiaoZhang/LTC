#version 450 core

// https://github.com/selfshadow/ltc_code/tree/master/webgl/shaders/ltc/ltc_quad.fs

layout(binding = 0, column_major) uniform _unused_name_uniform_buffer_global_layout_per_frame_binding
{
    // mesh
    highp mat4 model_transform;
    highp vec3 dcolor;
    highp float _padding_dcolor;
    highp vec3 scolor;
    highp float _padding_scolor;
    highp float roughness;
    highp float _padding_roughness_1;
    highp float _padding_roughness_2;
    highp float _padding_roughness_3;

    // camera
    highp mat4 view_transform;
    highp mat4 projection_transform;
    highp vec3 eye_position;
    highp float _padding_eye_position;

    // light
    highp vec4 rect_light_vetices[4];
    highp float intensity;
    highp float twoSided;
};

const highp float LUT_SIZE = 64.0;
const highp float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
const highp float LUT_BIAS = 0.5 / LUT_SIZE;

layout(binding = 1) uniform highp sampler2D ltc_1;
layout(binding = 2) uniform highp sampler2D ltc_2;

const highp float pi = 3.14159265;

layout(location = 0) in highp vec3 out_position;
layout(location = 1) in highp vec3 out_normal;

layout(location = 0) out highp vec4 out_color;

vec3 ToLinear(vec3 v);

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided);

float saturate(float v)
{
    return clamp(v, 0.0, 1.0);
}

void main()
{
    const highp vec3 points[4] = vec3[4](vec3(rect_light_vetices[0].xyz), vec3(rect_light_vetices[1].xyz), vec3(rect_light_vetices[2].xyz), vec3(rect_light_vetices[3].xyz));
    const highp vec3 lcol = vec3(intensity);

    highp vec3 world_position = out_position;
    highp vec3 world_normal = out_normal;
    highp vec3 dcol = ToLinear(dcolor);
    highp vec3 scol = ToLinear(scolor);

    highp vec3 pos = world_position;
    highp vec3 N = world_normal;
    highp vec3 V = normalize(eye_position - world_position);

    highp float ndotv = saturate(dot(N, V));
    highp vec2 uv = vec2(roughness, sqrt(1.0 - ndotv));
    uv = uv * LUT_SCALE + LUT_BIAS;

    highp vec4 t1 = texture(ltc_1, uv);
    highp vec4 t2 = texture(ltc_2, uv);

    highp mat3 Minv = mat3(
        vec3(t1.x, 0, t1.y),
        vec3(0, 1, 0),
        vec3(t1.z, 0, t1.w));

    highp vec3 spec = LTC_Evaluate(N, V, pos, Minv, points, (twoSided > 0.0));
    // BRDF shadowing and Fresnel
    spec *= scol * t2.x + (1.0 - scol) * t2.y;

    highp vec3 diff = LTC_Evaluate(N, V, pos, mat3(1), points, (twoSided > 0.0));

    highp vec3 col = lcol * (spec + dcol * diff);

    out_color = vec4(col, 1.0);
}

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float gamma = 2.2;

vec3 ToLinear(vec3 v)
{
    return PowVec3(v, gamma);
}

vec3 mul(mat3 m, vec3 v)
{
    return m * v;
}

mat3 mul(mat3 m1, mat3 m2)
{
    return m1 * m2;
}

#if 1
vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    // acos(dot(v1, v2)) * normalized(cross(v1, v2)) * (1 / 2PI)
    // = acos(dot(v1, v2)) * cross(v1, v2) 
    // = (acos(dot(v1, v2)) * (1 / sin(acos(dot(v1, v2)))) * (1 / 2PI)) * cross(v1, v2)
    // = theta_sintheta * cross(v1, v2)

    // cubic rational fit

    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y; // 2PI is divided here
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5 * inversesqrt(max(1.0 - x * x, 1e-7)) - v;

    return cross(v1, v2) * theta_sintheta;
}

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided)
{
    bool behind = (dot((points[0].xyz - P), (cross(points[1] - points[0], points[2] - points[0]))) < 0.0);
    if (behind && !twoSided)
    {
        float sum = 0.0;
        vec3 Lo_i = vec3(sum, sum, sum);
        return Lo_i;
    }
    else
    {
        // transform the vertices to the tangent space of the current shading position
        highp vec3 vertices_tangent_space[4];
        {
            highp vec3 T1 = normalize(V - N * dot(V, N));
            highp vec3 T2 = cross(N, T1);
            highp mat4 world_to_tangent_transform =
                mat4(
                    vec4(T1.x,        T2.x,        N.x,        0.0), // col 0
                    vec4(T1.y,        T2.y,        N.y,        0.0), // col 1
                    vec4(T1.z,        T2.z,        N.z,        0.0), // col 2
                    vec4(dot(T1, -P), dot(T2, -P), dot(N, -P), 1.0)  // col 3
                );
            vertices_tangent_space[0] = (world_to_tangent_transform * vec4(points[0], 1.0)).xyz;
            vertices_tangent_space[1] = (world_to_tangent_transform * vec4(points[1], 1.0)).xyz;
            vertices_tangent_space[2] = (world_to_tangent_transform * vec4(points[2], 1.0)).xyz;
            vertices_tangent_space[3] = (world_to_tangent_transform * vec4(points[3], 1.0)).xyz;
        }

        // LT "linear transform"
        highp vec3 L[4] = vec3[4](Minv * vertices_tangent_space[0], Minv * vertices_tangent_space[1], Minv * vertices_tangent_space[2], Minv * vertices_tangent_space[3]);

        // integrate
        L[0] = normalize(L[0]);
        L[1] = normalize(L[1]);
        L[2] = normalize(L[2]);
        L[3] = normalize(L[3]);

        vec3 vsum = vec3(0.0);

        vsum += IntegrateEdgeVec(L[0], L[1]);
        vsum += IntegrateEdgeVec(L[1], L[2]);
        vsum += IntegrateEdgeVec(L[2], L[3]);
        vsum += IntegrateEdgeVec(L[3], L[0]);

        float len = length(vsum);
        float z = vsum.z / len;

        if (behind)
            z = -z;

        vec2 uv = vec2(z * 0.5 + 0.5, len);
        uv = uv * LUT_SCALE + LUT_BIAS;

        float scale = texture(ltc_2, uv).w;

        float sum = len * scale;

        vec3 Lo_i = vec3(sum, sum, sum);
        return Lo_i;
    }
}

#else
vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5 * inversesqrt(max(1.0 - x * x, 1e-7)) - v;

    return cross(v1, v2) * theta_sintheta;
}

float IntegrateEdge(vec3 v1, vec3 v2)
{
    return IntegrateEdgeVec(v1, v2).z;
}

void ClipQuadToHorizon(inout vec3 L[5], out int n)
{
    // detect clipping config
    int config = 0;
    if (L[0].z > 0.0)
        config += 1;
    if (L[1].z > 0.0)
        config += 2;
    if (L[2].z > 0.0)
        config += 4;
    if (L[3].z > 0.0)
        config += 8;

    // clip
    n = 0;

    if (config == 0)
    {
        // clip all
    }
    else if (config == 1) // V1 clip V2 V3 V4
    {
        n = 3;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 2) // V2 clip V1 V3 V4
    {
        n = 3;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 3) // V1 V2 clip V3 V4
    {
        n = 4;
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
        L[3] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 4) // V3 clip V1 V2 V4
    {
        n = 3;
        L[0] = -L[3].z * L[2] + L[2].z * L[3];
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
    }
    else if (config == 5) // V1 V3 clip V2 V4) impossible
    {
        n = 0;
    }
    else if (config == 6) // V2 V3 clip V1 V4
    {
        n = 4;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 7) // V1 V2 V3 clip V4
    {
        n = 5;
        L[4] = -L[3].z * L[0] + L[0].z * L[3];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 8) // V4 clip V1 V2 V3
    {
        n = 3;
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
        L[1] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] = L[3];
    }
    else if (config == 9) // V1 V4 clip V2 V3
    {
        n = 4;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[2].z * L[3] + L[3].z * L[2];
    }
    else if (config == 10) // V2 V4 clip V1 V3) impossible
    {
        n = 0;
    }
    else if (config == 11) // V1 V2 V4 clip V3
    {
        n = 5;
        L[4] = L[3];
        L[3] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 12) // V3 V4 clip V1 V2
    {
        n = 4;
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
    }
    else if (config == 13) // V1 V3 V4 clip V2
    {
        n = 5;
        L[4] = L[3];
        L[3] = L[2];
        L[2] = -L[1].z * L[2] + L[2].z * L[1];
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
    }
    else if (config == 14) // V2 V3 V4 clip V1
    {
        n = 5;
        L[4] = -L[0].z * L[3] + L[3].z * L[0];
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
    }
    else if (config == 15) // V1 V2 V3 V4
    {
        n = 4;
    }

    if (n == 3)
        L[3] = L[0];
    if (n == 4)
        L[4] = L[0];
}

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided)
{
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = mul(Minv, transpose(mat3(T1, T2, N)));

    // polygon (allocate 5 vertices for clipping)
    vec3 L[5];
    L[0] = mul(Minv, points[0] - P);
    L[1] = mul(Minv, points[1] - P);
    L[2] = mul(Minv, points[2] - P);
    L[3] = mul(Minv, points[3] - P);

    // integrate
    float sum = 0.0;

    int n;
    ClipQuadToHorizon(L, n);

    if (n == 0)
        return vec3(0, 0, 0);
    // project onto sphere
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);
    L[4] = normalize(L[4]);

    // integrate
    sum += IntegrateEdge(L[0], L[1]);
    sum += IntegrateEdge(L[1], L[2]);
    sum += IntegrateEdge(L[2], L[3]);
    if (n >= 4)
        sum += IntegrateEdge(L[3], L[4]);
    if (n == 5)
        sum += IntegrateEdge(L[4], L[0]);

    sum = twoSided ? abs(sum) : max(0.0, sum);

    vec3 Lo_i = vec3(sum, sum, sum);

    return Lo_i;
}

#endif