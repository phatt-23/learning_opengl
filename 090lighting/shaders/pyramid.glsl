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
    OV_NormalVec3 = mat3(transpose(inverse(U_ModelMat4))) * AV_NormalVec3;
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

// Output the fragment color.
out vec4 OF_FragmentColor;

void main() {
    float ambience = 0.2f;

    vec3 normal = normalize(OV_NormalVec3);
    vec3 lightDirection = normalize(U_LightPositionVec3 - OV_CurrentPositionVec3);
    float diffussion = max(dot(normal, lightDirection), 0.0f);

    // Dummy use of U_CameraPositionVec3 to ensure it remains active
    float specularStrength = 0.5f;
    vec3 viewDirection = normalize(U_CameraPositionVec3 - OV_CurrentPositionVec3);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.f), 8.f);
    float speculant = specularAmount * specularStrength;

    OF_FragmentColor = texture(U_Texture0, OV_TextureCoordinatesVec2) * U_LightColorVec4 * (ambience + diffussion + speculant);
    OF_FragmentColor = vec4(vec3(OF_FragmentColor), 1.f);
}

