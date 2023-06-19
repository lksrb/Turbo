#type vertex
#version 450 core

// Attributes
layout(location = 0) in vec3 a_Position;
//layout(location = 1) out vec4 a_Color;
//layout(location = 2) in int a_EntityID;
//layout(location = 3) in mat4 a_InstanceMatrix;  // Per-instance transformation matrix

// Camera's view projection
layout(set = 0, binding = 0) uniform Camera 
{
    mat4 ViewProjection;
} u_Camera;

// Struct holding output data to fragment shader
struct VertexOutput
{
	vec4 Color;
};

// Outputs
layout (location = 0) out VertexOutput Output;
layout (location = 1) out flat int o_EntityID;

void main()
{
    Output.Color = vec4(0.2, 0.7, 0.5, 1.0);
	o_EntityID = -1;
    gl_Position = u_Camera.ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

// Struct holding input data (should match VertexOutput struct)
struct VertexInput
{
	vec4 Color;
};

// Inputs
layout(location = 0) in VertexInput Input;
layout(location = 1) in flat int in_EntityID;

// Outputs
layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = Input.Color;
}