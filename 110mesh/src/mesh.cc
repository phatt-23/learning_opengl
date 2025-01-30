//
// Created by phatt on 1/28/25.
//
module;

#include "std.h"
#include <GL/glew.h>

export module mesh;

import vertex_array;
import vertex_buffer;
import vertex_buffer.vertex_struct;
import index_buffer;
import texture;
import shader_program;
import camera;

/// Mesh represenst one drawable object. 
/// It consists of a VAO and textures.
/// It can be drawn with some shader 
/// in some camera's view coordinates.
export class Mesh {
private:
    // This is not needed.
    std::vector<Vertex> vertices;
    // This is not needed.
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    VertexArray vertexArray;
public:
    /// The constructor needs the vector of `vertices`, `indices` and `textures`.
    /// But the only vector it actually needs to store is the `textures`
    explicit Mesh(
        const std::vector<Vertex>& vertices,
        const std::vector<GLuint>& indices,
        const std::vector<Texture>& textures
    ) : vertices(vertices), indices(indices), textures(textures) {
        VertexBuffer vbo(vertices);
        IndexBuffer ibo(indices);
        vertexArray.linkVertexBufferAndIndexBuffer(vbo, Vertex::getLayout(), ibo);
    }

    ~Mesh() = default;

    /// Deletes its VAO and the textures.
    /// TODO: The textures can be used by many meshes 
    /// so I don't know if it should be deleting the textures.
    auto deleteResource() -> void {
        vertexArray.deleteResource(); 
        for (auto& texture : textures) {
            texture.deleteResource();
        }
    }

    auto getVertexArray() -> VertexArray& {
        return vertexArray;         
    }

    /// Draws out the mesh using specified shader program 
    /// with respect to the camera's point of view.
    auto draw(ShaderProgram& shader, const Camera& camera) -> void {
        int diffuseNumber = 0;
        int specularNumber = 0;

        for (std::size_t i = 0; i < textures.size(); i++) {
            const auto slot = i;

            const std::string type = texture::TypeToString(textures[i].getType());
            const int number = [&] {
                switch (textures[i].getType()) {
                    case texture::Type::DiffuseMap: { return diffuseNumber++; } break;
                    case texture::Type::SpecularMap: { return specularNumber++; } break;
                    default: throw std::runtime_error("Unknown texture type");
                }
            }();

            const std::string uniformName = "U_Material." + type + std::to_string(number);
            Texture::setSamplerInShader(shader, uniformName, slot);

            // textures[i].bindToLast();
            textures[i].bindToSlot(slot);
        }

        camera.sendPositionToShader(shader, "U_CameraPositionVec3");
        camera.sendProjectionViewMatToShader(shader, "U_CameraProjViewMat4");

        shader.bind();
        vertexArray.bind();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
        VertexArray::unbind();
        ShaderProgram::unbind();
    }
};

