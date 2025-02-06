//
// Created by phatt on 05/02/2025
//
module;

#include "std.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "stb_image.h"

export module skybox;

import vertex_buffer;
import vertex_buffer.layout;
import index_buffer;
import vertex_array;
import texture;
import camera;
import shader_program;

const ShaderProgramSource skyboxSources = {
    .vertexSource = R""""(
        /// #shader vertex
        #version 330 core

        layout(location = 0) in vec3 aPos;
        uniform mat4 cameraProjView;
        out vec3 TexCoords;

        void main() {
            TexCoords = aPos;
            vec4 pos = cameraProjView * vec4(aPos, 1.0);
            gl_Position = pos.xyww;
        }
    )"""",
    .fragmentSource = R""""(
        /// #shader fragment
        #version 330 core

        in vec3 TexCoords;
        uniform samplerCube skybox;
        out vec4 FragColor;

        void main() {
            FragColor = texture(skybox, TexCoords);
        }
    )"""",
};

const std::vector<float> skyboxVertices = {
    // position
    -1.0f, -1.0f,  1.0f, 
	-1.0f, -1.0f, -1.0f, 
	 1.0f, -1.0f, -1.0f, 
	 1.0f, -1.0f,  1.0f, 
	-1.0f,  1.0f,  1.0f, 
	-1.0f,  1.0f, -1.0f, 
	 1.0f,  1.0f, -1.0f, 
	 1.0f,  1.0f,  1.0f, 
};
const std::vector<GLuint> skyboxIndices = {
    0, 1, 2, 0, 2, 3, // Front face
    0, 4, 7, 0, 7, 3, // Left face
    3, 7, 6, 3, 6, 2, // Right face
    2, 6, 5, 2, 5, 1, // Back face
    1, 5, 4, 1, 4, 0, // Bottom face
    4, 5, 6, 4, 6, 7, // Top face
};


/// Is a 'mesh' for the skybox.
/// The draw call only needs a camera.
/// It doesn't need shader or transformation.
/// Shader code is hard-coded.
export class Skybox { // : public Mesh {
private:
    ShaderProgram mShader;
    VertexBuffer mVBO;
    IndexBuffer mIBO;
    VertexArray mVAO;
    Texture mTexture; // Texture holds the path
public:
    Skybox(const std::string& skyboxTexturesDirectory)
    : mShader(skyboxSources), mTexture(skyboxTexturesDirectory, 
                                        texture::Type::CubeMap),
        mVBO(skyboxVertices.data(), skyboxVertices.size()),
        mIBO(skyboxIndices),
        mVAO(mVBO, VertexBufferLayout().pushAttribute<float>(3, "pos"), mIBO)
    {}

    /// Drawing it last is more efficient because the shader doesn't
    /// have to run for pixel. The vertex shader must updated tho.
    /// And also the depth function must be GL_LEQUAL instead of GL_LESS.
    void draw(const Camera& camera, bool isDrawnLast = false) {
        // if the skybox is drawn inbetween disable 
        // depth testing and make the furthest.
        if (isDrawnLast) {
            glDepthFunc(GL_LEQUAL);
        } else {
            glDepthMask(GL_FALSE);
        }


        mTexture.bindToSlot(0); // bind to texture unit 0 for the shader to use it

        // bind shader and pass
        mShader.bind();
        mShader.setUniform1i("skybox", 0); // use the texture unit 0 as samplerCube
        mShader.setUniformMat4f("cameraProjView", 
                                            // getting rid of the translation by nulling out 
                                            // the translation in the right most column.
            camera.getProjectionMatrix() * glm::mat4(glm::mat3(camera.getViewMatrix())) 
        );

        mVAO.bind();

        glDrawElements(GL_TRIANGLES, static_cast<int>(skyboxIndices.size()), GL_UNSIGNED_INT, nullptr);

        VertexArray::unbind();
        ShaderProgram::unbind();


        if (isDrawnLast) {
            glDepthFunc(GL_LESS);
        } else {
            glDepthMask(GL_TRUE);
        }
    }
};
