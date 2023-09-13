#type vertex
#version 450 core

// Attributes

// Per vertex
layout(location = 0) in vec3 a_VertexPosition;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

// Per instance
layout(location = 3) in vec4 a_TransformRow0;
layout(location = 4) in vec4 a_TransformRow1;
layout(location = 5) in vec4 a_TransformRow2;
layout(location = 6) in vec4 a_TransformRow3;
layout(location = 7) in int a_EntityID;

void main()
{

}
