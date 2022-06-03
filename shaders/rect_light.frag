#version 450 core

layout(binding = 0, column_major) uniform _unused_name_uniform_buffer_global_layout_per_frame_binding
{
    // camera
    highp mat4 view_transform;
    highp mat4 projection_transform;

    // light
    highp vec4 rect_light_vetices[4];
    highp float intensity;
};

layout(location = 0) out highp vec4 out_color;

void main()
{
    const highp vec3 lcol = vec3(intensity);

    highp vec3 col = lcol;
    out_color = vec4(col, 1.0);
}