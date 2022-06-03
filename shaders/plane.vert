#version 450 core

layout(binding = 0, column_major) uniform _unused_name_uniform_buffer_global_layout_per_frame_binding
{
    // mesh
    highp mat4 model_transform;
	highp vec3 dcolor;
	highp float _padding_dcolor;
	highp vec3 scolor;
	highp float _padding_scolor;
	highp float roughness;
    highp float _padding_roughness_1;
    highp float _padding_roughness_2;
    highp float _padding_roughness_3;

    // camera
    highp mat4 view_transform;
    highp mat4 projection_transform;
    highp vec3 eye_position;
    highp float _padding_eye_position;

    // light
    highp vec4 rect_light_vetices[4];
    highp float intensity;
    highp float twoSided;
};

layout(location = 0) in highp vec3 in_position;
layout(location = 1) in highp vec3 in_normal;

layout(location = 0) out highp vec3 out_position;
layout(location = 1) out highp vec3 out_normal;

void main()
{
    highp vec3 model_position = in_position;
    highp vec3 world_position = (model_transform * vec4(model_position, 1.0)).xyz;
    highp vec4 clip_position = projection_transform * view_transform * vec4(world_position, 1.0);


    highp vec3 model_normal = in_normal;
    // TODO: normal transform
    highp mat3x3 tangent_transform = mat3x3(model_transform[0].xyz, model_transform[1].xyz, model_transform[2].xyz);
    highp vec3 world_normal = normalize(tangent_transform * model_normal);

    out_position = world_position;
    out_normal = world_normal;
    gl_Position = clip_position;
}