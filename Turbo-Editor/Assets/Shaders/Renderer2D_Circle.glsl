#type vertex
#version 450 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;
layout(location = 5) in int a_EntityID;

layout(set = 0, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int o_EntityID;

void main()
{
	Output.LocalPosition = a_LocalPosition;
	Output.Thickness = a_Thickness;
	Output.Color = a_Color;
	Output.Fade = a_Fade;

	o_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}

#type fragment
#version 450 core

struct VertexInput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
};

// Inputs
layout (location = 0) in VertexInput Input;
layout (location = 4) in flat int in_EntityID;

// Outputs
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

void main()
{
    // Calculate distance and fill circle with white
    float distance = 1.0 - length(Input.LocalPosition);
    float circleAlpha = smoothstep(0.0, Input.Fade, distance);
    circleAlpha *= smoothstep(Input.Thickness + Input.Fade, Input.Thickness, distance);

	if(circleAlpha == 0.0)
		discard;

    // Set output color
    o_Color = Input.Color;
	o_Color.a *= circleAlpha;

	if(circleAlpha > 0.0)
		o_EntityID = in_EntityID;
}