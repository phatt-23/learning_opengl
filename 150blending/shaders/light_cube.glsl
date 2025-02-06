/// #shader vertex
#version 330 core

// automatically when binding VAO
layout(location = 0) in vec3 AV_PositionVec3; 

uniform mat4 U_ModelMat4; // obtained in CPU draw function
uniform mat4 U_CameraProjViewMat4; // obtained in CPU draw function

void main() {
    gl_Position = U_CameraProjViewMat4 * U_ModelMat4 * vec4(AV_PositionVec3, 1.f);
}

/// #shader fragment
#version 330 core

uniform vec4 U_LightColorVec4; // manually set

out vec4 OF_FragmentColorVec4;

void main() {
    OF_FragmentColorVec4 = U_LightColorVec4;
}
