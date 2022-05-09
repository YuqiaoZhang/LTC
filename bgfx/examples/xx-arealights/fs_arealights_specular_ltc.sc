$input v_normal, v_tangent, v_wpos, v_shadowcoord, v_texcoord0

#include "common.sh"

#define LTC_NUM_POINTS 4
#define LTC_TEXTURED   1
#include "ltc.sh"

// -------------------------------------------------------------------------------------------------
void main()
{
    ShadingContext s = InitShadingContext(v_normal, v_tangent, v_wpos, u_viewPosition.xyz, v_texcoord0);

    vec2 coords = LTC_Coords(dot(s.n, s.o), s.roughness);
    mat3 Minv   = LTC_Matrix(s_texLTCMat, coords);
    vec3 Lo_i   = LTC_Evaluate(s.n, s.o, s.p, Minv, u_quadPoints, u_twoSided, s_texFilteredMap);

    // scale by light intensity
    Lo_i *= u_lightIntensity;

    // apply BRDF scale terms (BRDF magnitude and Schlick Fresnel)
    vec2 schlick = texture2D(s_texLTCAmp, coords).xy;
    Lo_i *= s.specColor*schlick.x + (1.0 - s.specColor)*schlick.y;

    // normalize
    Lo_i /= 2.0f * M_PI;

    // set output
    gl_FragColor = vec4(Lo_i, 1.0);
}
