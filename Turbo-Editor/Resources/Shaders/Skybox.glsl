#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

// Camera's view projection
layout(std140, set = 0, binding = 0) uniform Camera 
{
    mat4 u_ViewProjection;
    mat4 u_InverseViewProjection;
    mat4 u_InversedViewMatrix;
};

layout(location = 0) out vec3 o_Position;

void main()
{
    vec4 position = vec4(a_Position.xy, 1.0, 1.0);
    gl_Position = position;
    o_Position = (u_InverseViewProjection * position).xyz;
}

#type fragment
#version 450 core

layout(location = 0) in vec3 in_Position;

layout(binding = 1) uniform samplerCube u_TextureCube;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID_UNUSED; // Matches the attachments

void main()
{
    o_Color = texture(u_TextureCube, vec3(in_Position.xyz));
    o_EntityID_UNUSED = -1;
}