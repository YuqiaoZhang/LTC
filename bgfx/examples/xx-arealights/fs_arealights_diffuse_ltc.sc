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
    mat3 Minv   = identity33(); // identity matrix
    vec3 Lo_i   = LTC_Evaluate(s.n, s.o, s.p, Minv, u_quadPoints, u_twoSided, s_texFilteredMap);

    // scale by light intensity
    Lo_i *= u_lightIntensity;

    // scale by diffuse albedo
    Lo_i *= s.diffColor * u_albedo.rgb;

    // normalize
    Lo_i /= 2.0f * M_PI;

    // set output
    gl_FragColor = vec4(Lo_i, 1.0);
}
