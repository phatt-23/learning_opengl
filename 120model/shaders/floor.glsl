/// #shader vertex /////////////////////////////////////////////////////////////////////////////
#version 330 core

// variables marked as `in` are called attribute variables
// variables that are not used can be deleted out by the compiler while optimizing the code
layout(location = 0) in vec3 AV_PositionVec3; // Attribute variable in the Vertex shader
layout(location = 1) in vec3 AV_NormalVec3;
layout(location = 2) in vec3 AV_ColorVec3; // this one is included because we work with it in the main function
layout(location = 3) in vec2 AV_TextureCoordinatesVec2;
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

    // Sending the attribute variables further down the pipeline (to the fragment shader)
    OV_NormalVec3 = AV_NormalVec3;
    OV_ColorVec3 = AV_ColorVec3;
    OV_TextureCoordinatesVec2 = AV_TextureCoordinatesVec2;

    gl_Position = U_CameraProjViewMat4 * vec4(OV_CurrentPositionVec3, 1.f);
}

/// #shader fragment //////////////////////////////////////////////////////////////////////////
#version 330 core

#include "./std/lighting.glsl"
#include "./std/material.glsl"

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
uniform Material U_Material;

// Output the fragment color.
out vec4 OF_FragmentColorVec4;

void main() {
    OF_FragmentColorVec4 = pointLight(
        3.0f, 0.7f,
        U_LightColorVec4,
        U_LightPositionVec3,
        OV_CurrentPositionVec3,
        U_CameraPositionVec3,
        OV_NormalVec3,
        U_Material.DiffuseMap0,
        U_Material.SpecularMap0,
        OV_TextureCoordinatesVec2
    );

    OF_FragmentColorVec4 += directionalLight(
        vec3(1.0f, 1.0f, 0.0f),
        U_LightColorVec4,
        OV_CurrentPositionVec3,
        U_CameraPositionVec3,
        OV_NormalVec3,
        U_Material.DiffuseMap0,
        U_Material.SpecularMap0,
        OV_TextureCoordinatesVec2
    );

    OF_FragmentColorVec4 += spotLight(
        vec3(0.f, -1.f, 0.f), cos(radians(30.f)), cos(radians(40.f)),
        3.0f, 0.7f,
        U_LightColorVec4,
        U_LightPositionVec3,
        OV_CurrentPositionVec3,
        U_CameraPositionVec3,
        OV_NormalVec3,
        U_Material.DiffuseMap0,
        U_Material.SpecularMap0,
        OV_TextureCoordinatesVec2
    );
}


