//
// Created by phatt on 1/24/25.
//
module;

#include <GL/glew.h>

export module vertex_buffer;

export import vertex_buffer.supported_types;
export import vertex_buffer.layout;

export class VertexBuffer {
private:
    GLuint arrayBufferID = 0;
public:
    /// Given `data` and its size in bytes, it will construct a new VBO and copy the data to its data store.
    explicit VertexBuffer(const GLfloat* vertexAttributes, const std::uint32_t vertexAttributesCount) {
        // Generates new buffer object ID and set it to be GL_ARRAY_BUFFER.
        glGenBuffers(1, &arrayBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, arrayBufferID);
        // Creates new data store for the ARRAY_BUFFER object and copies the data.
        // If data is null the data store is created but not initialised.
        // Any pre-existing data in the buffer object's data store is deleted.
        const GLsizei dataSizeInBytes = static_cast<GLsizei>(vertexAttributesCount) * sizeof(GLfloat);
        glBufferData(GL_ARRAY_BUFFER, dataSizeInBytes, vertexAttributes, GL_DYNAMIC_DRAW);
        // Unbind it so no accidental overwrites can happen.
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /// Deletes the buffer and its data store.
    ~VertexBuffer() {
        glDeleteBuffers(1, &arrayBufferID);
    }

    auto bind() const -> void {
        glBindBuffer(GL_ARRAY_BUFFER, arrayBufferID);
    }

    static auto unbind() -> void {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};
