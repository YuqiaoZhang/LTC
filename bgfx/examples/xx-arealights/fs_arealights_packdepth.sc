$input v_position, v_texcoord0

#include "common.sh"

void main()
{
    float alpha = texture2D(s_albedoMap, v_texcoord0).a;
    if (alpha < 0.004)
    	discard;

    float depth = v_position.z/v_position.w * 0.5 + 0.5;
    gl_FragColor = packFloatToRgba(depth);
}
