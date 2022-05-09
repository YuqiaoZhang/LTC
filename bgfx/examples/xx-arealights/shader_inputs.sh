#ifndef SHADER_INPUTS
#define SHADER_INPUTS

uniform vec4 u_params0;
uniform vec4 u_params1;
uniform vec4 u_params2;

uniform vec4 u_quadPoints[4];
uniform vec4 u_starPoints[10];

uniform vec4 u_samples[NUM_SAMPLES];

uniform vec4 u_lightPosition;
uniform vec4 u_viewPosition;

uniform vec4 u_albedo;

SAMPLER2D(s_texLTCMat,      0);
SAMPLER2D(s_texLTCAmp,      1);
SAMPLER2D(s_texColorMap,    2);
SAMPLER2D(s_texFilteredMap, 3);

SAMPLER2D(s_albedoMap,      4);
SAMPLER2D(s_nmlMap,         5);
SAMPLER2D(s_roughnessMap,   6);
SAMPLER2D(s_metallicMap,    7);

#define u_F0              u_params0.x
#define u_roughness       u_params0.y
#define u_sampleCount     u_params0.w

#define u_lightIntensity  u_params1.x
#define u_twoSided       (u_params1.y > 0.0)

#endif