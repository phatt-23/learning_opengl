/// #shader vertex /////////////////////////////////////////////////////////////////////////////
#version 330 core

// obtained automatically by binding VAO
layout(location = 0) in vec3 AV_PositionVec3;

uniform mat4 U_ModelMat4; // obtained by mesh class in draw function
uniform mat4 U_CameraProjViewMat4; // obtained by mesh class in draw function

void main() {
    gl_Position = U_CameraProjViewMat4 * U_ModelMat4 * vec4(AV_PositionVec3, 1.f);
}

/// #shader fragment //////////////////////////////////////////////////////////////////////////
#version 330 core

out vec4 OF_FragmentColorVec4;

void main() {
    OF_FragmentColorVec4 = vec4(0.04, 0.28, 0.26, 1.0);
}


