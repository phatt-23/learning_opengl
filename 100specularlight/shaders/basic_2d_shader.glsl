/// #shader vertex
#version 330 core

// variables marked as `in` are called attribute variables
// variables that are not used can be deleted out by the compiler while optimizing the code
layout(location = 0) in vec3 AV_PositionVector; // Attribute variable in the Vertex shader
layout(location = 1) in vec3 AV_ColorVector; // this one is included because we work with it in the main function
layout(location = 2) in vec2 AV_TextureCoordinates;
out vec3 OV_ColorVector;
out vec2 OV_TextureCoordinates;

void main() {
    vec3 position = AV_PositionVector;
    gl_Position = vec4(position.x, position.y, position.z, 1.0f);

    OV_ColorVector = AV_ColorVector;
    OV_TextureCoordinates = AV_TextureCoordinates;
}

/// #shader fragment
#version 330 core

in vec3 OV_ColorVector;
in vec2 OV_TextureCoordinates;
out vec4 OF_FragmentColor;

// Tells OpenGL which texture unit it should use.
// It is an integer.
uniform sampler2D U_Texture0;

void main() {
    // OF_FragmentColor = vec4(OV_ColorVector, 1.0f);
    OF_FragmentColor = texture(U_Texture0, OV_TextureCoordinates);
}
