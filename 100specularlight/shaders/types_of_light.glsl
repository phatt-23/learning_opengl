/// #shader vertex /////////////////////////////////////////////////////////////////////////////
#version 330 core

// variables marked as `in` are called attribute variables
// variables that are not used can be deleted out by the compiler while optimizing the code
layout(location = 0) in vec3 AV_PositionVec3; // Attribute variable in the Vertex shader
layout(location = 1) in vec3 AV_ColorVec3; // this one is included because we work with it in the main function
layout(location = 2) in vec2 AV_TextureCoordinatesVec2;
layout(location = 3) in vec3 AV_NormalVec3;
// Uniform variables to specify MVP matrices.
// Describing how the object is oriented about its own axes and where it is in the world.
uniform mat4 U_ModelMat4;
// Cameras matrix describing where it is in the world and how it views the objects.
uniform mat4 U_CameraProjViewMat4;
// For the fragment shader.
out vec3 OV_ColorVec3;
out vec2 OV_TextureCoordinatesVec2;
out vec3 OV_NormalVec3;
out vec3 OV_CurrentPositionVec3;

void main() {
    OV_CurrentPositionVec3 = vec3(U_ModelMat4 * vec4(AV_PositionVec3, 1.f));

    gl_Position = U_CameraProjViewMat4 * vec4(OV_CurrentPositionVec3, 1.f);
    // Sending the attribute variables further down the pipeline (to the fragment shader)
    OV_ColorVec3 = AV_ColorVec3;
    OV_TextureCoordinatesVec2 = AV_TextureCoordinatesVec2;
    OV_NormalVec3 = AV_NormalVec3;
}

/// #shader fragment //////////////////////////////////////////////////////////////////////////
#version 330 core

// Get the variables from the vertex shader.
in vec3 OV_ColorVec3;
in vec2 OV_TextureCoordinatesVec2;
in vec3 OV_NormalVec3;
in vec3 OV_CurrentPositionVec3;

// Light source information
uniform vec4 U_LightColorVec4;
// Light source position
uniform vec3 U_LightPositionVec3;
// Eye position.
uniform vec3 U_CameraPositionVec3;
// Tells OpenGL which texture unit it should use. It is an integer.
uniform sampler2D U_Texture0;
uniform sampler2D U_Texture1;

// Output the fragment color.
out vec4 OF_FragmentColor;

/// The light source is so distant that the light rays emitted 
/// are parallel to one another. It has no dimming/decay of light intensity.
/// WARN: We want the direction to the the light source, not from the light source.
vec4 directionalLight(
    vec3 directionToTheLightSourceVec3, // You can tweak this.
    vec4 lightSourceColorVec4,
    vec3 currentPositionVec3,
    vec3 cameraPositionVec3,
    vec3 normalVec3,
    sampler2D diffuseTexture2D,
    sampler2D specularTexture2D,
    vec2 textureCoordinatesVec2
) {
    const float ambience = 0.2f;

    vec3 lightDirection = normalize(directionToTheLightSourceVec3);
    vec3 normal = normalize(normalVec3);
    float diffussion = max(dot(normal, lightDirection), 0.0f);

    float specularStrength = 0.5f;
    vec3 viewDirection = normalize(cameraPositionVec3 - currentPositionVec3);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.f), 16.f);
    float speculant = specularAmount * specularStrength;
    
    vec4 outColor;

    outColor = texture(diffuseTexture2D, textureCoordinatesVec2) * (ambience + diffussion);
    outColor += texture(specularTexture2D, textureCoordinatesVec2).r * speculant;
    outColor *= lightSourceColorVec4;
    outColor.w = 1.f;

    return outColor;
}

/// Takes into account the gradual decay of light intensity 
/// as the current position gets further away from the light source.
/// This 'curve' of decay can be tweaked by the parameters `a` and `b`.
///
/// A, B - constant parameters for tweaking the behaviour of the curve
/// i - light intensity
/// d - distance from the light source to the current pixel of the object
///
/// i = 1/(A*d^2 + B*d + 1)
/// 
vec4 pointLight(
    float a, float b, // You can tweak this.
    vec3 lightSourcePositionVec3,
    vec3 currentPositionVec3,
    vec3 cameraPositionVec3,
    vec3 normalVec3,
    sampler2D diffuseTexture2D,
    sampler2D specularTexture2D,
    vec2 textureCoordinatesVec2
) {
    vec3 lightDirectionNotNormalized = lightSourcePositionVec3 - currentPositionVec3;
    float distanceFromLight = length(lightDirectionNotNormalized);
    float lightAttenuation = 1.f / (a * pow(distanceFromLight, 2.f) + b * distanceFromLight + 1.f);

    const float ambience = 0.2f;

    vec3 normal = normalize(normalVec3);
    vec3 lightDirection = normalize(lightDirectionNotNormalized);
    float diffussion = max(dot(normal, lightDirection), 0.0f);

    float specularStrength = 0.5f;
    vec3 viewDirection = normalize(cameraPositionVec3 - currentPositionVec3);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.f), 8.f);
    float speculant = specularAmount * specularStrength;
    
    vec4 outColor;

    outColor = texture(diffuseTexture2D, textureCoordinatesVec2) * (diffussion * lightAttenuation + ambience);
    outColor += texture(specularTexture2D, textureCoordinatesVec2).r * (speculant * lightAttenuation);
    outColor *= U_LightColorVec4;
    outColor.w = 1.f;

    return outColor;
}

/// Represents a spotlight (flashlight) type of light.
/// This light is pointed at some direction ()
/// If has a circular shape with parametrised by outer
/// and inner angles, both are in their cosine values. 
/// The inner cosine says where the light starts to wither 
/// and the outer cosine says where it is completelly
/// cut off into darkness. Attenuation is also considered.
vec4 spotLight(
    vec3 lightSourcePointAtDirectionVec3, float innerConeCosine, float outerConeCosine, // You can tweak this.
    float a, float b, // You can tweak this.
    vec4 lightSourceColorVec4,
    vec3 lightSourcePositionVec3,
    vec3 currentPositionVec3,
    vec3 cameraPositionVec3,
    vec3 normalVec3,
    sampler2D diffuseTexture2D,
    sampler2D specularTexture2D,
    vec2 textureCoordinatesVec2
) {
    // The inner angle must be smaller than the outer angle.
    // Therefor the cosine of the inner angle must be greater
    // than the cosine of the outer angle. We don't want 
    // spotlights where the inner and outer angles are equal
    // so consider only when inner cosine is greater than the outer.
    // The early return reverses the logic.
    if (innerConeCosine <= outerConeCosine) {
        return vec4(0.f);
    }

    vec3 lightDirectionNotNormalized = lightSourcePositionVec3 - currentPositionVec3;
    float distanceFromLight = length(lightDirectionNotNormalized);
    float lightAttenuation = 1.f / (a * pow(distanceFromLight, 2.f) + b * distanceFromLight + 1.f);

    const float ambience = 0.2f;

    vec3 normal = normalize(normalVec3);
    vec3 lightDirection = normalize(lightDirectionNotNormalized);
    float diffussion = max(dot(normal, lightDirection), 0.0f);

    float specularStrength = 0.5f;
    vec3 viewDirection = normalize(cameraPositionVec3 - currentPositionVec3);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.f), 8.f);
    float speculant = specularAmount * specularStrength;

    // angle between the direction of where the spotlight 
    // is pointing to (lightPointAtDirection) and the 
    // direction of the light ray hitting the current 
    // position of the object (lightDirection).
    float angle = dot(lightSourcePointAtDirectionVec3, -lightDirection);
    float spotLightIntensity = clamp((angle - outerConeCosine)/(innerConeCosine - outerConeCosine), 0.f, 1.f);

    vec4 outColor;
    outColor = texture(diffuseTexture2D, textureCoordinatesVec2) * (diffussion * spotLightIntensity * lightAttenuation + ambience);
    outColor += texture(specularTexture2D, textureCoordinatesVec2).r * (speculant * spotLightIntensity * lightAttenuation);
    outColor *= lightSourceColorVec4;
    outColor.w = 1.f;

    return outColor;
}

void main() {
    OF_FragmentColor = pointLight(
        3.0f, 0.7f,
        U_LightPositionVec3,
        OV_CurrentPositionVec3,
        U_CameraPositionVec3,
        OV_NormalVec3,
        U_Texture0,
        U_Texture1,
        OV_TextureCoordinatesVec2
    );

    OF_FragmentColor += directionalLight(
        vec3(1.0f, 1.0f, 0.0f),
        U_LightColorVec4,
        OV_CurrentPositionVec3,
        U_CameraPositionVec3,
        OV_NormalVec3,
        U_Texture0,
        U_Texture1,
        OV_TextureCoordinatesVec2
    );

    OF_FragmentColor += spotLight(
        vec3(0.f, -1.f, 0.f), cos(radians(30.f)), cos(radians(40.f)),
        3.0f, 0.7f,
        U_LightColorVec4,
        U_LightPositionVec3,
        OV_CurrentPositionVec3,
        U_CameraPositionVec3,
        OV_NormalVec3,
        U_Texture0,
        U_Texture1,
        OV_TextureCoordinatesVec2
    );
}


