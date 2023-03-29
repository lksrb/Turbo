#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in int a_EntityID;

layout(set = 0, binding = 0) uniform Camera {
    mat4 ViewProjection;
} u_Camera;

struct VertexOutput
{
	vec4 Color;
};

layout (location = 0) out VertexOutput Output;
layout (location = 1) out flat int o_EntityID;

void main()
{
    Output.Color = a_Color;
	o_EntityID = a_EntityID;

    gl_Position = u_Camera.ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

struct VertexInput
{
	vec4 Color;
};

// Inputs
layout(location = 0) in VertexInput Input;
layout(location = 1) in flat int in_EntityID;

// Outputs
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

void main()
{
	o_EntityID = in_EntityID;
    o_Color = Input.Color;
}