#version 450 core

precision highp float;

layout(binding = 0, column_major) uniform _unused_name_buffer_uniform_per_frame
{
    //column_major
    mat4 view;

    vec2 resolution;
    float _padding_resolution_1;
    float _padding_resolution_2;

    vec3 dcolor;
    float _padding_dcolor;

    vec3 scolor;
    float _padding_scolor;

    float roughness;
    float intensity;
    float width;
    float height;

    float roty;
    float rotz;
    int twoSided;
};

layout(binding = 1) uniform sampler2D ltc_mat;
layout(binding = 2) uniform sampler2D ltc_mag;

layout(location = 0) out vec4 out_color;

const float LUT_SIZE = 64.0;
const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
const float LUT_BIAS = 0.5 / LUT_SIZE;

const float pi = 3.14159265;

// Tracing and intersection
///////////////////////////

struct Ray
{
    vec3 origin;
    vec3 dir;
};

struct Rect
{
    vec3 center;
    vec3 dirx;
    vec3 diry;
    float halfx;
    float halfy;

    vec4 plane;
};

bool RayPlaneIntersect(Ray ray, vec4 plane, out float t)
{
    t = -dot(plane, vec4(ray.origin, 1.0)) / dot(plane.xyz, ray.dir);
    return t > 0.0;
}

bool RayRectIntersect(Ray ray, Rect rect, out float t)
{
    bool intersect = RayPlaneIntersect(ray, rect.plane, t);
    if (intersect)
    {
        vec3 pos = ray.origin + ray.dir * t;
        vec3 lpos = pos - rect.center;

        float x = dot(lpos, rect.dirx);
        float y = dot(lpos, rect.diry);

        if (abs(x) > rect.halfx || abs(y) > rect.halfy)
            intersect = false;
    }

    return intersect;
}

// Camera functions
///////////////////

Ray GenerateCameraRay(vec2 xy)
{
    Ray ray;

    // Random jitter within pixel for AA
    ray.dir = normalize(vec3(xy, 2.0));

    float focalDistance = 2.0;
    float ft = focalDistance / ray.dir.z;
    vec3 pFocus = ray.dir * ft;

    ray.origin = vec3(0);
    ray.dir = normalize(pFocus - ray.origin);

    // Apply camera transform
    ray.origin = (view * vec4(ray.origin, 1)).xyz;
    ray.dir = (view * vec4(ray.dir, 0)).xyz;

    return ray;
}

vec3 mul(mat3 m, vec3 v)
{
    return m * v;
}

mat3 mul(mat3 m1, mat3 m2)
{
    return m1 * m2;
}

vec3 rotation_y(vec3 v, float a)
{
    vec3 r;
    r.x = v.x * cos(a) + v.z * sin(a);
    r.y = v.y;
    r.z = -v.x * sin(a) + v.z * cos(a);
    return r;
}

vec3 rotation_z(vec3 v, float a)
{
    vec3 r;
    r.x = v.x * cos(a) - v.y * sin(a);
    r.y = v.x * sin(a) + v.y * cos(a);
    r.z = v.z;
    return r;
}

vec3 rotation_yz(vec3 v, float ay, float az)
{
    return rotation_z(rotation_y(v, ay), az);
}

// Linearly Transformed Cosines
///////////////////////////////

float IntegrateEdge(vec3 v1, vec3 v2)
{
    float cosTheta = dot(v1, v2);
    float theta = acos(cosTheta);
    float res = cross(v1, v2).z * ((theta > 0.001) ? theta / sin(theta) : 1.0);

    return res;
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

vec3 LTC_Evaluate(
    vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], int twoSided)
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
    float sum = 0.0;

    sum += IntegrateEdge(L[0], L[1]);
    sum += IntegrateEdge(L[1], L[2]);
    sum += IntegrateEdge(L[2], L[3]);
    if (n >= 4)
        sum += IntegrateEdge(L[3], L[4]);
    if (n == 5)
        sum += IntegrateEdge(L[4], L[0]);

    sum = (twoSided != 0) ? abs(sum) : max(0.0, sum);

    vec3 Lo_i = vec3(sum, sum, sum);

    return Lo_i;
}

// Scene helpers
////////////////

void InitRect(out Rect rect)
{
    rect.dirx = rotation_yz(vec3(1, 0, 0), roty * 2.0 * pi, rotz * 2.0 * pi);
    rect.diry = rotation_yz(vec3(0, 1, 0), roty * 2.0 * pi, rotz * 2.0 * pi);

    rect.center = vec3(0, 6, 32);
    rect.halfx = 0.5 * width;
    rect.halfy = 0.5 * height;

    vec3 rectNormal = cross(rect.dirx, rect.diry);
    rect.plane = vec4(rectNormal, -dot(rectNormal, rect.center));
}

void InitRectPoints(Rect rect, out vec3 points[4])
{
    vec3 ex = rect.halfx * rect.dirx;
    vec3 ey = rect.halfy * rect.diry;

    points[0] = rect.center - ex - ey;
    points[1] = rect.center + ex - ey;
    points[2] = rect.center + ex + ey;
    points[3] = rect.center - ex + ey;
}

// Misc. helpers
////////////////

float saturate(float v)
{
    return clamp(v, 0.0, 1.0);
}

vec3 saturate(vec3 v)
{
    return vec3(saturate(v.x), saturate(v.y), saturate(v.z));
}

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
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

const float gamma = 2.2;

vec3 ToLinear(vec3 v) { return PowVec3(v, gamma); }
vec3 ToSRGB(vec3 v) { return PowVec3(v, 1.0 / gamma); }

void main()
{
    Rect rect;
    InitRect(rect);

    vec3 points[4];
    InitRectPoints(rect, points);

    vec4 floorPlane = vec4(0, 1, 0, 0);

    vec3 lcol = vec3(intensity);
    vec3 dcol = ToLinear(dcolor);
    vec3 scol = ToLinear(scolor);

    vec3 col = vec3(0);

    vec2 ray_xy = vec2(2.0, -2.0) * (gl_FragCoord.xy) / resolution + vec2(-1.0, 1.0);
    Ray ray = GenerateCameraRay(ray_xy);

    float distToFloor;
    bool hitFloor = RayPlaneIntersect(ray, floorPlane, distToFloor);
    if (hitFloor)
    {
        vec3 pos = ray.origin + ray.dir * distToFloor;

        vec3 N = floorPlane.xyz;
        vec3 V = -ray.dir;

        float theta = acos(dot(N, V));
        vec2 uv = vec2(roughness, theta / (0.5 * pi));
        uv = uv * LUT_SCALE + LUT_BIAS;

        vec4 t = texture(ltc_mat, uv);
        mat3 Minv = mat3(
            vec3(1, 0, t.y),
            vec3(0, t.z, 0),
            vec3(t.w, 0, t.x));

        vec3 spec = LTC_Evaluate(N, V, pos, Minv, points, twoSided);
        spec *= texture(ltc_mag, uv).r;

        vec3 diff = LTC_Evaluate(N, V, pos, mat3(1), points, twoSided);

        col = lcol * (scol * spec + dcol * diff);
        col /= 2.0 * pi;
    }

    float distToRect;
    if (RayRectIntersect(ray, rect, distToRect))
        if ((distToRect < distToFloor) || !hitFloor)
            col = lcol;
    
    col = aces_fitted(col);

    // framebuffer srgb
    // col = ToSRGB(col);

    out_color = vec4(col, 1.0);
}