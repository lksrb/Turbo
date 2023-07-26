#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in uint a_TexIndex;
layout(location = 4) in int a_EntityID;

layout(set = 0, binding = 0) uniform Camera 
{
    mat4 ViewProjection;
} u_Camera;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput Output;
layout (location = 2) out flat uint o_TexIndex;
layout (location = 3) out flat int o_EntityID;

void main()
{
    Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
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
};

// Inputs
layout(location = 0) in VertexInput Input;
layout(location = 2) in flat uint in_TexIndex;
layout(location = 3) in flat int in_EntityID;

layout(binding = 1) uniform sampler2D u_Textures[2];

float screenPxRange() {
	const float pxRange = 2.0; // set to distance field's pixel range
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_Textures[in_TexIndex], 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(Input.TexCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

// Outputs
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

void main()
{
	vec4 texColor = Input.Color * texture(u_Textures[in_TexIndex], Input.TexCoord);
	vec3 msd = texture(u_Textures[in_TexIndex], Input.TexCoord).rgb;
	float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
		if (opacity == 0.0)
			discard;

	vec4 bgColor = vec4(0.0);
    o_Color = mix(bgColor, Input.Color, opacity);
	o_EntityID = in_EntityID;

	if (o_Color.a == 0.0)
		discard;
}