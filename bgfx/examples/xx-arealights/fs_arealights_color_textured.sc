$input v_texcoord0

#include "common.sh"

void main()
{
    vec3 color = texture2D(s_texColorMap, v_texcoord0).rgb;
    color *= u_lightIntensity;

    // TODO: fix this test. Needs glFrontFace support or DX equivalent
    color = (!gl_FrontFacing || u_twoSided) ? color : vec3(0, 0, 0);

    gl_FragColor = vec4(color, 1);
}
