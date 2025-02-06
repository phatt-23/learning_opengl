/// #shader vertex /////////////////////////////////////////////////////////////////////////////
#version 330 core

// obtained automatically by binding VAO
layout(location = 0) in vec3 AV_PositionVec3; 
layout(location = 1) in vec3 AV_NormalVec3;
layout(location = 2) in vec2 AV_TextureCoordinatesVec2;
layout(location = 3) in vec3 AV_TangentVec3;
layout(location = 4) in vec3 AV_BitangentVec3;

uniform mat4 U_ModelMat4; // obtained by mesh class in draw function
uniform mat4 U_CameraProjViewMat4; // obtained by mesh class in draw function

out VS_OUT {
    vec3 OV_FragmentPositionVec3;
    vec3 OV_NormalVec3;
    vec2 OV_TextureCoordinatesVec2;
    vec3 OV_TangentVec3;
    vec3 OV_BitangentVec3;
} Out;

void main() {
    Out.OV_FragmentPositionVec3 = vec3(U_ModelMat4 * vec4(AV_PositionVec3, 1.f));

    Out.OV_NormalVec3 = normalize(transpose(inverse(mat3(U_ModelMat4))) * AV_NormalVec3);
    Out.OV_TextureCoordinatesVec2 = AV_TextureCoordinatesVec2;
    Out.OV_TangentVec3 = AV_TangentVec3;
    Out.OV_BitangentVec3 = AV_BitangentVec3;

    gl_Position = U_CameraProjViewMat4 * vec4(Out.OV_FragmentPositionVec3, 1.f);
}

/// #shader fragment //////////////////////////////////////////////////////////////////////////
#version 330 core

// #include "./floor_lighting.glsl"
#include "./std/lighting.glsl"
#include "./std/material.glsl"

in VS_OUT {
    vec3 OV_FragmentPositionVec3;
    vec3 OV_NormalVec3;
    vec2 OV_TextureCoordinatesVec2;
    vec3 OV_TangentVec3;
    vec3 OV_BitangentVec3;
} In;

uniform vec3 U_CameraPositionVec3; // obtained by mesh class in draw function 
uniform vec4 U_LightColorVec4; // passed manually
uniform vec3 U_LightPositionVec3; // passed manually
uniform Material U_Material; // obtained through draw function

out vec4 OF_FragmentColorVec4;

void main() {
    OF_FragmentColorVec4 = pointLight(
        0.0f, 0.0f, 33,
        U_LightColorVec4,
        U_LightPositionVec3,
        In.OV_FragmentPositionVec3,
        U_CameraPositionVec3,
        In.OV_NormalVec3,
        U_Material.DiffuseMap0,
        U_Material.SpecularMap0,
        In.OV_TextureCoordinatesVec2
    );

    // OF_FragmentColorVec4 += directionalLight(
    //     vec3(1.f, 1.f, 0.f), 
    //     32,
    //     vec4(1.f, 1.0f, 1.0f, 1.f),
    //     OV_FragmentPositionVec3,
    //     U_CameraPositionVec3,
    //     OV_NormalVec3,
    //     U_Material.DiffuseMap0,
    //     U_Material.SpecularMap0,
    //     OV_TextureCoordinatesVec2
    // );

    OF_FragmentColorVec4 += spotLight(
        vec3(0.f, -1.f, 1.f), 
        cos(radians(30.f)),
        cos(radians(45.f)), 
        1.0f, 0.7f,
        33,
        U_LightColorVec4,
        U_LightPositionVec3, 
        In.OV_FragmentPositionVec3,
        U_CameraPositionVec3,
        In.OV_NormalVec3,
        U_Material.DiffuseMap0,
        U_Material.SpecularMap0,
        In.OV_TextureCoordinatesVec2
    );
}


