#version 450 core

layout(location = 0) out highp vec2 out_uv;

void main()
{
    const vec2 positions[3] = vec2[3](vec2(3.0, 1.0), vec2(-1.0, 1.0), vec2(-1.0, -3.0));
    const vec2 uv[3] = vec2[3](vec2(2.0, 1.0), vec2(0.0, 1.0), vec2(0.0, -1.0));

    out_uv = uv[gl_VertexID];
    gl_Position = vec4(positions[gl_VertexID], 0.5, 1.0);
}