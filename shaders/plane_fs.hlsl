// https://github.com/selfshadow/ltc_code/tree/master/webgl/shaders/ltc/ltc_quad.fs

cbuffer _unused_name_uniform_buffer_global_layout_per_frame_binding : register(b0)
{
	// mesh
	column_major float4x4 model_transform;
	float3 dcolor;
	float _padding_dcolor;
	float3 scolor;
	float _padding_scolor;
	float roughness;
	float _padding_roughness_1;
	float _padding_roughness_2;
	float _padding_roughness_3;

	// camera
	column_major float4x4 view_transform;
	column_major float4x4 projection_transform;
	float3 eye_position;
	float _padding_eye_position;

	// light
	float4 rect_light_vetices[4];
	float intensity;
	float twoSided;
};

SamplerState ltc_lut_sampler : register(s0);
Texture2DArray ltc_matrix_lut : register(t0);
Texture2DArray ltc_norm_lut : register(t1);

float3 ToLinear(float3 v)
{
	return pow(v, 2.2);
}

#include "BRDF.hlsli"

void LTC_DECODE_GGX_LUT(float roughness, float NoV, out float3x3 linear_transform_inversed, out float n_d_norm, out float f_d_norm);

#include "LTC.hlsli"

void main(
	in float4 d3d_Position
	: SV_POSITION,
	  in float3 in_position
	: TEXCOORD0,
	  in float3 in_normal
	: TEXCOORD1,
	  out float4 out_color
	: SV_TARGET0)
{
	const float3 points[4] = {rect_light_vetices[0].xyz, rect_light_vetices[1].xyz, rect_light_vetices[2].xyz, rect_light_vetices[3].xyz};
	const float3 lcol = float3(intensity, intensity, intensity);

	float3 P = in_position;
	float3 N = in_normal;
	float3 V = normalize(eye_position - in_position);
	float3 diffuse_color = ToLinear(dcolor);
	float3 specular_color = ToLinear(scolor);

	const bool two_sided = twoSided > 0.0;

	float3 col = float3(0.0, 0.0, 0.0);
	if (two_sided)
	{
		if (EvaluateBRDFLTCLightAttenuation(P, points) > 0.0)
		{
			col += lcol * EvaluateBRDFLTC(diffuse_color, roughness, specular_color, P, N, V, points);
		}

		// The facing of the quad is determined by the winding order of the vertices.
		const float3 points_reverse[4] = {points[3], points[2], points[1], points[0]};
		if (EvaluateBRDFLTCLightAttenuation(P, points_reverse) > 0.0)
		{
			col += lcol * EvaluateBRDFLTC(diffuse_color, roughness, specular_color, P, N, V, points_reverse);
		}
	}
	else
	{
		if (EvaluateBRDFLTCLightAttenuation(P, points) > 0.0)
		{
			col += lcol * EvaluateBRDFLTC(diffuse_color, roughness, specular_color, P, N, V, points);
		}
	}

	out_color = float4(col, 1.0);
}

#define LTC_GGX_LUT_INDEX 0

void LTC_DECODE_GGX_LUT(float roughness, float NoV, out float3x3 linear_transform_inversed, out float n_d_norm, out float f_d_norm)
{
	float out_width;
	float out_height;
	float out_elements;
	float out_number_of_levels;
	ltc_matrix_lut.GetDimensions(0, out_width, out_height, out_elements, out_number_of_levels);

	float LUT_SIZE = out_width;
	float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
	float LUT_BIAS = 0.5 / LUT_SIZE;

	float2 lut_uv = LUT_BIAS + LUT_SCALE * float2(roughness, sqrt(1.0 - NoV));
	float4 ltc_ggx_matrix_lut_encoded = ltc_matrix_lut.SampleLevel(ltc_lut_sampler, float3(lut_uv, float(LTC_GGX_LUT_INDEX)), 0.0).rgba;
	float2 ltc_ggx_norm_lut_encoded = ltc_norm_lut.SampleLevel(ltc_lut_sampler, float3(lut_uv, float(LTC_GGX_LUT_INDEX)), 0.0).rg;

	linear_transform_inversed = float3x3(
		float3(ltc_ggx_matrix_lut_encoded.x, 0.0, ltc_ggx_matrix_lut_encoded.z), // row 0
		float3(0.0, 1.0, 0.0),													 // row 1
		float3(ltc_ggx_matrix_lut_encoded.y, 0.0, ltc_ggx_matrix_lut_encoded.w)  // row 2
	);

	n_d_norm = ltc_ggx_norm_lut_encoded.x;
	f_d_norm = ltc_ggx_norm_lut_encoded.y;
}