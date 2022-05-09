$input a_position, a_normal, a_tangent, a_texcoord0
$output  v_normal, v_tangent, v_wpos, v_shadowcoord, v_texcoord0

#include "../common/common.sh"

uniform mat4 u_lightMtx;
uniform vec4 u_params1;
#define u_shadowMapOffset u_params1.y

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

    vec4 normal = a_normal * 2.0 - 1.0;
    v_normal = normalize(mul(u_model[0], vec4(normal.xyz, 0.0)).xyz);

    vec4 tangent = a_tangent * 2.0 - 1.0;
    v_tangent = normalize(mul(u_model[0], vec4(tangent.xyz, 0.0)).xyz);

    v_texcoord0 = a_texcoord0;

    v_wpos = mul(u_model[0], vec4(a_position, 1.0)).xyz;

    vec3 posOffset = a_position + normal.xyz * u_shadowMapOffset;
    v_shadowcoord = mul(u_lightMtx, vec4(posOffset, 1.0));
}
