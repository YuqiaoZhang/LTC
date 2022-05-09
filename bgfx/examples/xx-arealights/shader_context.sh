#ifndef SHADER_CONTEXT
#define SHADER_CONTEXT

// -------------------------------------------------------------------------------------------------

struct ShadingContext
{
    vec3 n;
    vec3 o;
    vec3 p;

    vec3 diffColor;
    vec3 specColor;

    float roughness;
};

ShadingContext InitShadingContext(vec3 ng, vec3 tg, vec3 p, vec3 e, vec2 uv)
{
    ShadingContext s;
    s.p = p;
    s.o = normalize(e - p);

    vec3 bg = cross(ng, tg);
    mat3 t2w = mat3_from_columns(tg, bg, ng);

    vec3 n = FetchNormal(s_nmlMap, uv, t2w);
    s.n = CorrectNormal(n, s.o);

    vec3 baseColor = texture2D(s_albedoMap, uv).rgb;
    float metallic = texture2D(s_metallicMap, uv).r;
    s.diffColor = baseColor*(1.0 - metallic);
    s.specColor = mix(vec3(u_F0, u_F0, u_F0), baseColor, metallic);

    float roughness    = texture2D(s_roughnessMap, uv).r;
    float nmlRoughness = texture2D(s_nmlMap, uv).b;
    roughness = pow(pow(roughness, 4.0) + pow(nmlRoughness, 4.0), 0.25);
    s.roughness = max(roughness*u_roughness, MIN_ROUGHNESS);

    return s;
}

#endif
