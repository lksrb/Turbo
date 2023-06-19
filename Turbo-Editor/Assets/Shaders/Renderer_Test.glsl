#type vertex
#version 450 core

const vec3 g_Positions[6] = vec3[6](
    // Front face
    vec3(-0.5, -0.5,  0.0),   // Vertex 0
    vec3( 0.5, -0.5,  0.0),   // Vertex 1
    vec3( 0.5,  0.5,  0.0),   // Vertex 2
    vec3( 0.5,  0.5,  0.0),   // Vertex 3
    vec3(-0.5,  0.5,  0.0),   // Vertex 4
    vec3(-0.5, -0.5,  0.0)    // Vertex 5
);

void main()
{
     float rotationAngle = 3.14 / 4;  // Adjust the rotation angle as desired

    float cosAngle = cos(rotationAngle);
    float sinAngle = sin(rotationAngle);

    mat3 rotationMatrix = mat3(
        cosAngle, -sinAngle, 0.0,
        sinAngle, cosAngle, 0.0,
        0.0, 0.0, 1.0
    );

    gl_Position = vec4(rotationMatrix * g_Positions[gl_VertexIndex], 1.0);
}

#type fragment
#version 450 core

// Outputs
layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = vec4(0.8, 0.2, 0.3, 1.0);
}