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

    /// Delete the VAO but not its references to the VBO or IBO.
    ~VertexArray() {
        glDeleteVertexArrays(1, &vertexArrayID);
    }

    /// Links the VBO its layout and IBO. This class does not keep any references to them only OpenGL does.
    auto linkVertexBufferWithIndices(const VertexBuffer &buffer, const VertexBufferLayout &layout, const IndexBuffer &indices) const -> void {
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

