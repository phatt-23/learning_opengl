//
// Created by phatt on 1/24/25.
//
module;

#include "std.h"
#include <GL/glew.h>

export module vertex_buffer;

export import vertex_buffer.supported_types;
export import vertex_buffer.layout;
export import vertex_buffer.vertex_struct;

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

    /// Given an dynamic array of vertices, it will create
    /// a vertex buffer object and associate the data with it.
    explicit VertexBuffer(const std::vector<Vertex>& vertices) {
        glGenBuffers(1, &arrayBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, arrayBufferID);
        const GLsizei dataSizeInBytes = static_cast<GLsizei>(vertices.size()) * sizeof(Vertex);
        glBufferData(GL_ARRAY_BUFFER, dataSizeInBytes, vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /// Deconstructor that doesn't delete the the 
    /// array buffer because that's a resource of OpenGL.
    ~VertexBuffer() = default;

    /// Deletes the buffer and its data store.
    auto deleteResource() -> void {
        glDeleteBuffers(1, &arrayBufferID);
        arrayBufferID = 0;
    }

    auto bind() const -> void {
        glBindBuffer(GL_ARRAY_BUFFER, arrayBufferID);
    }

    static auto unbind() -> void {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};
