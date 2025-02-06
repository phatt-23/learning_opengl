//
// Created by phatt on 1/24/25.
//

module;

#include "std.h"
#include <GL/glew.h>
#include <glm/glm.hpp>

export module vertex_buffer.supported_types;
// WARN: If you decide to add a new supported data type, 
// you must add it to both mapping functions.

/// Compile-time OpenGL data type mapping to OpenGL data type macro codes.
/// Only maps supported data types, if they are not supported, static assertion is raised.
export template<typename DataType>
auto getGLTypeMacroCode() -> int {
    if constexpr (std::is_same_v<DataType, GLfloat>) return GL_FLOAT;
    if constexpr (std::is_same_v<DataType, GLint>) return GL_INT;
    if constexpr (std::is_same_v<DataType, GLuint>) return GL_UNSIGNED_INT;
    if constexpr (std::is_same_v<DataType, GLubyte>) return GL_UNSIGNED_BYTE;
    if constexpr (std::is_same_v<DataType, glm::mat4>) return GL_FLOAT_MAT4;

    static_assert(true, "VertexBuffer unsupported data type");
}

/// Returns the sizeof OpenGL data type given its macro code.
/// Only maps supported data types, if they are not supported, -1 is returned.
export auto getSizeOfGLTypeFromMacroCode(const GLuint openGLMacroCode) -> GLint {
    switch (openGLMacroCode) {
        case GL_FLOAT: return sizeof(GLfloat);
        case GL_INT: return sizeof(GLint);
        case GL_UNSIGNED_INT: return sizeof(GLuint);
        case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
        default: return -1;
    }
}

/// Concept that check if VertexBuffer supports the given data type.
export template <typename DataType>
concept IsSupportedByVertexBuffer = requires() {
    // Requires the called function to return. 
    // If the type is not supported, static assertion is raised.
    getGLTypeMacroCode<DataType>();
};
