//
// Created by phatt on 1/24/25.
//
module;

#include "std.h"
#include <GL/glew.h>

export module index_buffer;

export class IndexBuffer {
private:
    GLuint elementArrayBufferID = 0;
    GLsizei elementCount = 0;
public:
    /// Given `indices` and its size in bytes, it will construct a new IBO and copy the data to its data store.
    explicit IndexBuffer(const GLuint* indices, const std::uint32_t indicesCount)
    : elementCount(static_cast<GLsizei>(indicesCount)) {
        // Generates new buffer object ID and set it to be GL_ELEMENT_ARRAY_BUFFER.
        glGenBuffers(1, &elementArrayBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBufferID);
        // Creates new data store for the ARRAY_BUFFER object and copies the data.
        // If data is null the data store is created but not initialised.
        // Any pre-existing data in the buffer object's data store is deleted.
        const GLsizei indicesSizeInBytes = static_cast<GLsizei>(indicesCount) * sizeof(GLuint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSizeInBytes, indices, GL_DYNAMIC_DRAW);
        // Unbind it so no accidental overwrites can happen.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    explicit IndexBuffer(const std::vector<GLuint>& indices)
    : elementCount(static_cast<GLsizei>(indices.size())) {
        glGenBuffers(1, &elementArrayBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBufferID);
        const GLsizei indicesSizeInBytes = static_cast<GLsizei>(indices.size()) * sizeof(GLuint);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSizeInBytes, indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    /// Deconstructor that doesn't delete the 
    /// the element array buffer because that
    /// a resource of OpenGL.
    ~IndexBuffer() = default;
    
    /// Deletes the element array buffer from OpenGL.
    auto deleteResource() -> void {
        glDeleteBuffers(1, &elementArrayBufferID);
        elementArrayBufferID = 0;
    }

    [[nodiscard]] auto getElementCount() const -> GLsizei {
        return elementCount;
    }

    auto bind() const -> void {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBufferID);
    }

    static auto unbind() -> void {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};
