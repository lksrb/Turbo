#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in uint a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;

layout(set = 0, binding = 0) uniform Camera {
    mat4 ViewProjection;
} u_Camera;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
	float TilingFactor;
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat uint o_TexIndex;
layout (location = 4) out flat int o_EntityID;

void main()
{
    Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	Output.TilingFactor = a_TilingFactor;
	o_EntityID = a_EntityID;

	o_TexIndex = a_TexIndex;

    gl_Position = u_Camera.ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

struct VertexInput
{
	vec4 Color;
	vec2 TexCoord;
	float TilingFactor;
};

// Inputs
layout(location = 0) in VertexInput Input;
layout(location = 3) in flat uint in_TexIndex;
layout(location = 4) in flat int in_EntityID;

layout(binding = 1) uniform sampler2D u_Textures[32];

// Outputs
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

void main()
{
	o_EntityID = in_EntityID;

	vec4 test = vec4(1.0,0.0,0.0,1.0);
    o_Color = Input.Color * texture(u_Textures[in_TexIndex], Input.TexCoord * Input.TilingFactor);
}