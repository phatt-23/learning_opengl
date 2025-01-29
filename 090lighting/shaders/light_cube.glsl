/// #shader vertex
#version 330 core

layout(location = 0) in vec3 AV_PositionVec3;

uniform mat4 U_ModelMat4;
uniform mat4 U_CameraProjViewMat4;

void main() {
    gl_Position = U_CameraProjViewMat4 * U_ModelMat4 * vec4(AV_PositionVec3, 1.f);
}

/// #shader fragment
#version 330 core

out vec4 OF_FragmentColorVec4;

uniform vec4 U_ColorVec4;

void main() {
    OF_FragmentColorVec4 = U_ColorVec4;
}
