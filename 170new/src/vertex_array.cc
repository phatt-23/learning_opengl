//
// Created by phatt on 1/24/25.
//
module;

#include <GL/glew.h>

export module vertex_array;

import vertex_buffer;
import index_buffer;

export class VertexArray {
private:
    GLuint vertexArrayID = 0;
public:
    /// Generate new vertex array object ID.
    explicit VertexArray() {
        glGenVertexArrays(1, &vertexArrayID);
    }

    /// Generate new vertex array object ID and links VBO, layout of the VBO and IBO.
    explicit VertexArray(const VertexBuffer &buffer, const VertexBufferLayout &layout, const IndexBuffer &indices) {
        glGenVertexArrays(1, &vertexArrayID);
        linkVertexBufferAndIndexBuffer(buffer, layout, indices);
    }

    /// Generate new vertex array object ID and links VBO and layout of the VBO.
    explicit VertexArray(const VertexBuffer &buffer, const VertexBufferLayout &layout) {
        glGenVertexArrays(1, &vertexArrayID);
        linkVertexBuffer(buffer, layout);
    }


    /// Deconstructor that doesn't delete the
    /// the vertex array resource from OpenGL.
    ~VertexArray() = default;

    /// Delete the VAO but not its references to the VBO or IBO.
    auto deleteResource() -> void {
        glDeleteVertexArrays(1, &vertexArrayID);
        vertexArrayID = 0;
    }

    /// Links the VBO its layout and IBO. This class does not keep any references to them only OpenGL does.
    auto linkVertexBufferAndIndexBuffer(const VertexBuffer &buffer, const VertexBufferLayout &layout, const IndexBuffer &indices) const -> void {
        // Bind VAO.
        bind();
        // Bind VBO and IBO.
        buffer.bind();
        indices.bind();
        // Configures the layout.
        layout.configure();
        // Unbind VAO.
        unbind();
        // Unbind VBO and IBO.
        VertexBuffer::unbind();
        IndexBuffer::unbind();
    }

    /// Links the VBO and its layout. This class does not keep any references to the VBO only OpenGL does.
    auto linkVertexBuffer(const VertexBuffer &buffer, const VertexBufferLayout &layout) const -> void {
        // Bind the VAO.
        bind();
        // Bind the VBO that the VAO will use.
        buffer.bind();
        // Configures the layout.
        layout.configure();
        // Unbind the VAO.
        unbind();
        // Unbind the VBO.
        VertexBuffer::unbind();
    }

    /// Binds the VAO.
    auto bind() const -> void {
        glBindVertexArray(vertexArrayID);
    }

    /// Unbinds the VAO.
    static auto unbind() -> void {
        glBindVertexArray(0);
    }
};

