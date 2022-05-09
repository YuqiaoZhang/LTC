#ifndef SHADER_HELPERS
#define SHADER_HELPERS

// -------------------------------------------------------------------------------------------------

mat3 transpose(mat3 v)
{
    mat3 tmp;
    tmp[0] = vec3(v[0].x, v[1].x, v[2].x);
    tmp[1] = vec3(v[0].y, v[1].y, v[2].y);
    tmp[2] = vec3(v[0].z, v[1].z, v[2].z);

    return tmp;
}

mat3 identity33()
{
    mat3 tmp;
    tmp[0] = vec3(1, 0, 0);
    tmp[1] = vec3(0, 1, 0);
    tmp[2] = vec3(0, 0, 1);

    return tmp;
}

mat3 mat3_from_columns(vec3 c0, vec3 c1, vec3 c2)
{
    mat3 m = mat3(c0, c1, c2);
#if BGFX_SHADER_LANGUAGE_HLSL
    // The HLSL matrix constructor takes rows rather than columns, so transpose after
    m = transpose(m);
#endif
    return m;
}

mat3 mat3_from_rows(vec3 c0, vec3 c1, vec3 c2)
{
    mat3 m = mat3(c0, c1, c2);
#if !BGFX_SHADER_LANGUAGE_HLSL
    m = transpose(m);
#endif
    return m;
}

// -------------------------------------------------------------------------------------------------

int modi(int x, int y)
{
    return int(mod(x, y));
}

mat3 BasisFrisvad(vec3 v)
{
    vec3 x, y;

    if (v.z < -0.999999)
    {
        x = vec3( 0, -1, 0);
        y = vec3(-1,  0, 0);
    }
    else
    {
        float a = 1.0 / (1.0 + v.z);
        float b = -v.x*v.y*a;
        x = vec3(1.0 - v.x*v.x*a, b, -v.x);
        y = vec3(b, 1.0 - v.y*v.y*a, -v.y);
    }

    return mat3_from_columns(x, y, v);
}

// From: https://briansharpe.wordpress.com/2011/11/15/a-fast-and-simple-32bit-floating-point-hash-function/
vec4 FAST_32_hash(vec2 gridcell)
{
    // gridcell is assumed to be an integer coordinate
    const vec2 OFFSET = vec2(26.0, 161.0);
    const float DOMAIN = 71.0;
    const float SOMELARGEFLOAT = 951.135664;
    vec4 P = vec4(gridcell.xy, gridcell.xy + vec2(1, 1));
    P = P - floor(P * (1.0 / DOMAIN)) * DOMAIN;    //    truncate the domain
    P += OFFSET.xyxy;                              //    offset to interesting part of the noise
    P *= P;                                        //    calculate and return the hash
    return fract(P.xzxz * P.yyww * (1.0 / SOMELARGEFLOAT));
}

// -------------------------------------------------------------------------------------------------

float GGX(vec3 V, vec3 L, float alpha, out float pdf)
{
    if (V.z <= 0.0 || L.z <= 0.0)
    {
        pdf = 0.0;
        return 0.0;
    }

    float a2 = alpha*alpha;

    // height-correlated Smith masking-shadowing function
    float G1_wi = 2.0*V.z/(V.z + sqrt(a2 + (1.0 - a2)*V.z*V.z));
    float G1_wo = 2.0*L.z/(L.z + sqrt(a2 + (1.0 - a2)*L.z*L.z));
    float G     = G1_wi*G1_wo / (G1_wi + G1_wo - G1_wi*G1_wo);

    // D
    vec3 H = normalize(V + L);
    float d = 1.0 + (a2 - 1.0)*H.z*H.z;
    float D = a2/(M_PI * d*d);

    float ndoth = H.z;
    float vdoth = dot(V, H);

    if (vdoth <= 0.0)
    {
        pdf = 0.0;
        return 0.0;
    }

    pdf = D * ndoth / (4.0*vdoth);

    float res = D * G / 4.0 / V.z / L.z;
    return res;
}

// -------------------------------------------------------------------------------------------------

bool QuadRayTest(vec4 q[4], vec3 pos, vec3 dir, out vec2 uv, bool twoSided)
{
    // compute plane normal and distance from origin
    vec3 xaxis = q[1].xyz - q[0].xyz;
    vec3 yaxis = q[3].xyz - q[0].xyz;

    float xlen = length(xaxis);
    float ylen = length(yaxis);
    xaxis = xaxis / xlen;
    yaxis = yaxis / ylen;

    vec3 zaxis = normalize(cross(xaxis, yaxis));
    float d = -dot(zaxis, q[0].xyz);

    float ndotz = -dot(dir, zaxis);
    if (twoSided)
        ndotz = abs(ndotz);

    if (ndotz < 0.00001)
        return false;

    // compute intersection point
    float t = -(dot(pos, zaxis) + d) / dot(dir, zaxis);

    if (t < 0.0)
        return false;

    vec3 projpt = pos + dir * t;

    // use intersection point to determine the UV
    uv = vec2(dot(xaxis, projpt - q[0].xyz),
              dot(yaxis, projpt - q[0].xyz)) / vec2(xlen, ylen);

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        return false;

    return true;
}

vec3 FetchDiffuseFilteredTexture(sampler2D texLightFiltered, vec3 p1_, vec3 p2_, vec3 p3_, vec3 p4_)
{
    // area light plane basis
    vec3 V1 = p2_ - p1_;
    vec3 V2 = p4_ - p1_;
    vec3 planeOrtho = (cross(V1, V2));
    float planeAreaSquared = dot(planeOrtho, planeOrtho);
    float planeDistxPlaneArea = dot(planeOrtho, p1_);
    // orthonormal projection of (0,0,0) in area light space
    vec3 P = planeDistxPlaneArea * planeOrtho / planeAreaSquared - p1_;

    // find tex coords of P
    float dot_V1_V2 = dot(V1,V2);
    float inv_dot_V1_V1 = 1.0 / dot(V1, V1);
    vec3 V2_ = V2 - V1 * dot_V1_V2 * inv_dot_V1_V1;
    vec2 Puv;
    Puv.y = dot(V2_, P) / dot(V2_, V2_);
    Puv.x = dot(V1, P)*inv_dot_V1_V1 - dot_V1_V2*inv_dot_V1_V1*Puv.y ;

    // LOD
    float d = abs(planeDistxPlaneArea) / pow(planeAreaSquared, 0.75);

    return texture2DLod(texLightFiltered, vec2(0.125, 0.125) + 0.75 * Puv, log(2048.0*d)/log(3.0) ).rgb;
}

vec3 FetchNormal(sampler2D nmlSampler, vec2 texcoord, mat3 t2w)
{
    vec3 n = texture2D(nmlSampler, texcoord).wyz*2.0 - 1.0;

    // Recover z
    n.z = sqrt(max(1.0 - n.x*n.x - n.y*n.y, 0.0));

    return normalize(mul(t2w, n));
}

// See section 3.7 of
// "Linear Efficient Antialiased Displacement and Reflectance Mapping: Supplemental Material"
vec3 CorrectNormal(vec3 n, vec3 v)
{
    if (dot(n, v) < 0.0)
        n = normalize(n - 1.01*v*dot(n, v));
    return n;
}

#endif