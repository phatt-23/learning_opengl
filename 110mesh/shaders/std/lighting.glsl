#include "material.glsl"

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
    vec4 lightColorSourceVec4,
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
    outColor *= lightColorSourceVec4;
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

