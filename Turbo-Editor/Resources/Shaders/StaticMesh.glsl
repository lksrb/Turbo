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

// Camera's view projection
layout(std140, set = 0, binding = 0) uniform Camera 
{
    mat4 u_ViewProjection;
    mat4 u_InversedViewProjection;
    mat4 u_InversedViewMatrix;
};

// NOTE: This can hold 256 bytes for NVIDIA GTX 1050Ti
// NOTE: Minimum supported by any driver should be 128 bytes
//layout(push_constant) uniform PushConstantTest
//{
//    mat4 ViewProjection;
//} p_PushConstantTest;

// Struct holding output data to fragment shader
struct VertexOutput
{
    vec3 WorldPosition;
    vec3 Normal;
    vec3 ViewPosition;
    vec2 TexCoords;
};

// Outputs
layout (location = 0) out VertexOutput Output;
layout (location = 5) out flat int o_EntityID;

void main()
{
    // Model matrix
    mat4 transform = mat4(
        a_TransformRow0, 
        a_TransformRow1, 
        a_TransformRow2, 
        a_TransformRow3);

    // NOTE: Inversing matrices is expensive
    // Should be done on the CPU side VVVVVVVVV
    Output.Normal = mat3(transpose(inverse(transform))) * a_Normal;
    Output.WorldPosition = vec3(transform * vec4(a_VertexPosition, 1.0));
    Output.ViewPosition = vec3(u_InversedViewMatrix * vec4(Output.WorldPosition, 1.0));
    Output.TexCoords = a_TexCoord;
    o_EntityID = a_EntityID;

    gl_Position = u_ViewProjection * transform * vec4(a_VertexPosition, 1.0);
}

#type fragment
#version 450 core

// Struct holding input data (should match VertexOutput struct)
struct VertexInput
{
    vec3 WorldPosition;
    vec3 Normal;
    vec3 ViewPosition;
    vec2 TexCoord;
};

// Inputs
layout(location = 0) in VertexInput Input;
layout(location = 5) in flat int in_EntityID;

// Outputs
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct DirectionalLight
{
    vec4 Direction;
    vec3 Radiance;
    float Intensity;
};

struct PointLight
{
    vec4 Position;
    vec3 Radiance;
    
    float Intensity;
    float Radius;
    float FallOff;
};

struct SpotLight 
{
    vec4 Position;
    vec4 Direction;
    vec3 Radiance;

    float Intensity;
    float InnerCone;
    float OuterCone;
};

// 0 - Diffuse, ambient
// 1 - Specular
layout(binding = 1) uniform sampler2D u_MaterialTexture[2];

layout(push_constant) uniform Material
{
    vec3 AlbedoColor;
    //float Roughness;
} p_Material;

// Point lights
// TODO: Reduce the amount of active point lights by calculating which light is visible and which is not
// std140 - buffer layout must be multiplier of 16 to match C++ struct
layout(std140, binding = 2) uniform LightEnvironment
{
    DirectionalLight u_DirectionalLights[64];
    uint u_DirectionalLightCount;

    PointLight u_PointLights[64];
    uint u_PointLightCount;

    SpotLight u_SpotLights[64];
    uint u_SpotLightCount;
};

// Point light calculation
vec3 CalculatePointLights(vec3 normal, vec3 viewDir, float shininess)
{
    vec3 result = vec3(0.0);

    for(int i = 0; i < u_PointLightCount; i++)
    {
        PointLight light = u_PointLights[i];

        vec3 lightDir = normalize(vec3(light.Position) - Input.WorldPosition);
        float lightDistance = length(vec3(light.Position) - Input.WorldPosition);

        // Calculate diffuse
        float diffuseAngle = max(dot(normal, lightDir), 0.0);

        // Calculate specular
        vec3 reflectDir = reflect(-lightDir, normal); // Phong
        vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong
        float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
        
        // Attenuation
        float constant = 1.0;
        float linear = 0.09;
        float quadratic = 0.032;

        // Calculate attenuation
        float attenuation = clamp(1.0 - (lightDistance * lightDistance) / (light.Radius * light.Radius), 0.0, 1.0);
        attenuation *= mix(attenuation, 1.0, light.FallOff);

        // TODO: Materials
        vec3 lightAmbient = vec3(0.05, 0.05, 0.05);
        vec3 lightDiffuse = vec3(0.8, 0.8, 0.8);
        vec3 lightSpecular = vec3(1.0, 1.0, 1.0);

        // Combine results
        vec3 ambient  = light.Intensity * lightAmbient * vec3(texture(u_MaterialTexture[0], Input.TexCoord));
        vec3 diffuse  = light.Radiance * light.Intensity * lightDiffuse  * diffuseAngle * vec3(texture(u_MaterialTexture[0], Input.TexCoord));
        vec3 specular = light.Radiance * light.Intensity * lightSpecular * spec * vec3(texture(u_MaterialTexture[1], Input.TexCoord));

        ambient  *= attenuation;
        diffuse  *= attenuation;
        specular *= attenuation; 

        result += (ambient + diffuse + specular);
    }
    return result;
}

// Directional light calculation
vec3 CalculateDirectionalLights(vec3 normal, vec3 viewDir, float shininess)
{
    vec3 result = vec3(0.0);

    for(int i = 0; i < u_DirectionalLightCount; i++)
    {
        DirectionalLight light = u_DirectionalLights[i];

        // Calculate direction of the light source
        vec3 lightDir = normalize(-vec3(light.Direction));

        // Calculate diffuse
        float diffuseAngle = max(dot(normal, lightDir), 0.0);

        // Calculate specular 
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        // TODO: Materials
        vec3 lightAmbient = vec3(0.5, 0.5, 0.5);
        vec3 lightDiffuse = vec3(0.8, 0.8, 0.8);
        vec3 lightSpecular = vec3(1.0, 1.0, 1.0);

        // Combine results
        vec3 ambient  = light.Intensity * lightAmbient * light.Radiance * vec3(texture(u_MaterialTexture[0], Input.TexCoord));
        vec3 diffuse  = light.Intensity * lightDiffuse * light.Radiance * diffuseAngle * vec3(texture(u_MaterialTexture[0], Input.TexCoord));
        vec3 specular = light.Intensity * lightSpecular * light.Radiance *  spec * vec3(texture(u_MaterialTexture[1], Input.TexCoord));

        result += (ambient + diffuse + specular);
    }

    return result;
}

vec3 CalculateSpotLights(vec3 normal, vec3 viewDir, float shininess)
{
    vec3 result = vec3(0.0);

    for(int i = 0; i < u_SpotLightCount; i++)
    {
        SpotLight light = u_SpotLights[i];
        vec3 lightDir = normalize(vec3(light.Position) - Input.WorldPosition);
        float theta = dot(lightDir, vec3(normalize(-light.Direction)));

        // Smooth edges
        float epsilon = light.InnerCone - light.OuterCone;
        float interpolatedIntensity = clamp((theta - light.OuterCone) / epsilon, 0.0, 1.0);

        // Calculate diffuse
        float diffuseAngle = max(dot(normal, lightDir), 0.0);
        
        // Calculate specular 
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        
        // TODO: Materials
        vec3 lightAmbient = vec3(0.05, 0.05, 0.05);
        vec3 lightDiffuse = vec3(0.8, 0.8, 0.8);
        vec3 lightSpecular = vec3(1.0, 1.0, 1.0);
        
        //vec3 radiance = vec3(0.8f, 0.0f, 0.0f);

        // Combine results
        vec3 ambient  = light.Intensity * interpolatedIntensity * lightAmbient * vec3(texture(u_MaterialTexture[0], Input.TexCoord));
        vec3 diffuse  = vec3(light.Radiance) * light.Intensity * interpolatedIntensity * lightDiffuse  * diffuseAngle * vec3(texture(u_MaterialTexture[0], Input.TexCoord));
        vec3 specular = vec3(light.Radiance) * light.Intensity * interpolatedIntensity * lightSpecular * spec * vec3(texture(u_MaterialTexture[1], Input.TexCoord));
    
        result += ambient + diffuse + specular;
    }

    return result;
}

void main()
{
    vec3 normal = normalize(Input.Normal);
    vec3 viewDir = normalize(Input.ViewPosition - Input.WorldPosition);
    float shininess = 32.0;

    // Phase 1: Directional light
    vec3 result = CalculateDirectionalLights(normal, viewDir, shininess);

    // Phase 2: Point lights
    result += CalculatePointLights(normal, viewDir,  shininess);

    // Phase 3: Spot light
    result += CalculateSpotLights(normal, viewDir, shininess);

    o_Color = vec4(result, 1.0);
    o_EntityID = in_EntityID;
}
