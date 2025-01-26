/// #shader vertex
#version 330 core

// variables marked as `in` are called attribute variables
// variables that are not used can be deleted out by the compiler while optimizing the code
layout(location = 0) in vec3 AV_PositionVec3; // Attribute variable in the Vertex shader
layout(location = 1) in vec3 AV_ColorVec3; // this one is included because we work with it in the main function
layout(location = 2) in vec2 AV_TextureCoordinatesVec2;
// Uniform variables to specify MVP matrices.
// Describing how the object is oriented about its own axes and where it is in the world.
uniform mat4 U_ModelViewMat4;
// Cameras matrix describing where it is in the world and how it views the objects.
uniform mat4 U_CameraViewProjectionMat4;
// For the fragment shader.
out vec3 OV_ColorVec3;
out vec2 OV_TextureCoordinatesVec2;

void main() {
    gl_Position = U_CameraViewProjectionMat4 * U_ModelViewMat4 * vec4(AV_PositionVec3, 1.0f);
    // Sending the attribute variables further down the pipeline (to the fragment shader)
    OV_ColorVec3 = AV_ColorVec3;
    OV_TextureCoordinatesVec2 = AV_TextureCoordinatesVec2;
}

/// #shader fragment
#version 330 core
// Get the variables from the vertex shader.
in vec3 OV_ColorVec3;
in vec2 OV_TextureCoordinatesVec2;
// Tells OpenGL which texture unit it should use.
// It is an integer.
uniform sampler2D U_Texture0;
// Output the fragment color.
out vec4 OF_FragmentColor;

void main() {
    // OF_FragmentColor = vec4(OV_ColorVec3, 1.0f);
    OF_FragmentColor = texture(U_Texture0, OV_TextureCoordinatesVec2);
}
