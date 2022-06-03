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

void main()
{
    highp vec3 world_position = rect_light_vetices[gl_VertexID].xyz;
    highp vec4 clip_position = projection_transform * view_transform * vec4(world_position, 1.0);

    gl_Position = clip_position;
}