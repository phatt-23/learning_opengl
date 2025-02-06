/* ------------------------------ COMPILE_VERT ----------------------------- */
#shader vertex
#version 330 core

layout(location = 0) in vec3 AV_PositionVec3; 
layout(location = 1) in vec3 AV_NormalVec3;
layout(location = 2) in vec2 AV_TextureCoordinatesVec2;
layout(location = 3) in vec3 AV_TangentVec3;
layout(location = 4) in vec3 AV_BitangentVec3;

uniform mat4 U_ModelMat4;
uniform mat4 U_CameraProjViewMat4;

out vec2 OV_TextureCoordinatesVec2;

void main()
{
    gl_Position = U_CameraProjViewMat4 * U_ModelMat4 * vec4(AV_PositionVec3, 1.0);
    OV_TextureCoordinatesVec2 = AV_TextureCoordinatesVec2;    
}

/* ------------------------------ COMPILE_FRAG ------------------------------ */
#shader fragment
#version 330 core

in vec2 OV_TextureCoordinatesVec2;

struct Material {
    sampler2D DiffuseMap0;
    sampler2D DiffuseMap1;
    sampler2D DiffuseMap2;
    sampler2D DiffuseMap3;
    sampler2D DiffuseMap4;
    sampler2D DiffuseMap5;
    sampler2D DiffuseMap6;
    sampler2D DiffuseMap7;

    sampler2D SpecularMap0;
    sampler2D SpecularMap1;
    sampler2D SpecularMap2;
    sampler2D SpecularMap3;
    sampler2D SpecularMap4;
    sampler2D SpecularMap5;
    sampler2D SpecularMap6;
    sampler2D SpecularMap7;
};

uniform Material U_Material;

out vec4 OF_FragmentColor;

void main()
{    
    OF_FragmentColor = texture2D(U_Material.DiffuseMap0, OV_TextureCoordinatesVec2);
}

