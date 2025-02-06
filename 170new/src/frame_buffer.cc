//
// Created by phatt on 06/02/2025
//
module;

#include "std.h"
#include <GL/glew.h>
#include <glm/glm.hpp>

export module frame_buffer;

import vertex_buffer.vertex_struct;
import vertex_buffer;
import index_buffer;
import vertex_array;
import texture;
import shader_program;
import transformation;

export namespace framebuffer {
    enum Attachment : uint8_t {
        // ColorTexture        = 0b00000001,       // no renderbuffer for color buffer
        DepthTexture        = 0b00000010,       
        DepthRenderBuffer   = 0b00000100,
        StencilTexture      = 0b00001000,     
        StencilRenderBuffer = 0b00010000,
    };
};

using namespace framebuffer;


// USAGE:
//
// FrameBuffer fb({width, height}, {1.0, 1.0, 1.0, 1.0}, 
//                framebuffer::Attachment::ColorTexture 
//                | framebuffer::Attachment::StencilRenderBuffer 
//                | framebuffer::Attachment::DepthRenderBuffer
// );
//
// fb.draw(start: {x, y}, size: {width, height});

const std::vector<Vertex> quadVertices{
    Vertex{ .position = {-1, -1,  0 }, .texUV = {0, 0} },
    Vertex{ .position = { 1, -1,  0 }, .texUV = {1, 0} },
    Vertex{ .position = { 1,  1,  0 }, .texUV = {1, 1} },
    Vertex{ .position = {-1,  1,  0 }, .texUV = {0, 1} }
};

const std::vector<GLuint> quadIndices{ 0, 1, 2, 0, 2, 3, };

/// ColorAttach = Texture
/// Depth = Texture of RenderBuffer
/// Stencil = Texture of RenderBuffer
export class FrameBuffer {
private:
    GLuint mFrameBufferID;
    Texture mColorTexture;
    glm::u32vec2 mSize;
    glm::vec4 mClearColor;
    std::uint32_t mAttachmentFlags;

    VertexBuffer mVBO;
    IndexBuffer mIBO;
    VertexArray mVAO;
public:
    explicit FrameBuffer(
        const glm::vec2& size,
        const glm::vec4& clearColor = glm::vec4(1.0, 0.1, 0.1, 1.0), 
        const uint32_t attachmentFlags = DepthRenderBuffer | StencilRenderBuffer
    ) 
    : mSize(size)
    , mClearColor(clearColor)
    , mAttachmentFlags(attachmentFlags)
    , mColorTexture(size, texture::Type::DiffuseMap)
    , mVBO(quadVertices)
    , mIBO(quadIndices)
    , mVAO(mVBO, Vertex::getLayout(), mIBO)
    { 
        glGenFramebuffers(1, &mFrameBufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                               GL_TEXTURE_2D, mColorTexture.getID(), 0);
        
        if (mAttachmentFlags & (DepthRenderBuffer | StencilRenderBuffer)) {
            unsigned int renderBufferID;
            glGenRenderbuffers(1, &renderBufferID);
            glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mSize.x, mSize.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                                      GL_RENDERBUFFER, renderBufferID);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        } else {
            throw std::runtime_error("Don't know how to handle anything else yet.\n");
        }

        // Check if the framebuffer has all it needs.
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer creation not complete.\n");
        }

        // Unbind it for now (reverting back to the default framebuffer).
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
   
    /// Bind back to the default framebuffer.
    static auto bindToDefault() -> void {
        std::cout << "Bound default framebuffer with id: 0\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /// Clears the current framebuffer's buffers (color, depth, stencil).
    static auto clear(const std::uint32_t bufferBits, const glm::vec4& clearColor) -> void {
        std::cout << "Cleared buffers: " << bufferBits
                << ", clear color: (" << clearColor.x << ", "
                                    << clearColor.y << ", "
                                    << clearColor.z << ", "
                                    << clearColor.w << ")\n";
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(bufferBits); 
    }

    /// Binds this framebuffer to be current.
    auto bind() const -> void {
        std::cout << "Bound framebuffer with id: " << mFrameBufferID << "\n";
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID);
    }

    /// Binds this framebuffer and clears this framebuffer's buffers.
    auto clear(const std::uint32_t bufferBits) const -> void {
        bind();
        FrameBuffer::clear(bufferBits, mClearColor);
    }
    
    auto getID() const -> GLuint {
        return mFrameBufferID;
    }

    auto getColorTexture() -> const Texture& {
        return mColorTexture;
    }

    auto getVAO() const -> const VertexArray& {
        return mVAO; 
    }

    auto getIBO() const -> const IndexBuffer& {
        return mIBO;
    }

    auto draw(
        ShaderProgram& shader, 
        const Transformation& transform
    ) -> void {
        std::cout << "Drawing the framebuffer " << mFrameBufferID << " to currently bound framebuffer\n";

        // Go back to the default framebuffer and draw the color buffer
        // of the previous framebuffer. 
        // FrameBuffer::bindToDefault();
        // Disable the depth testing so the texture always appears on top. 
        // glDisable(GL_DEPTH_TEST);
        // We clear only the color buffer because thats all we use in the default framebuffer
        // when drawing.
        // glClearColor(1.0, 1.0, 1.0, 1.0);
        // glClear(GL_COLOR_BUFFER_BIT);

        // Bind the texture to be drawn out.
        const int textureUnitSlot = 0;

        mColorTexture = Texture("./textures/brick.png", texture::Type::DiffuseMap);

        mColorTexture.bindToSlot(textureUnitSlot);

        // Bind the simple shader that only draws out the texture.
        shader.bind();
        shader.setUniform1i("U_ScreenTexture", textureUnitSlot);
        shader.setUniformMat4f("U_ModelMat4", transform.getModelMat());

        // Draw it.
        mVAO.bind();
        glDrawElements(GL_TRIANGLES, static_cast<int>(mIBO.getElementCount()), 
                       GL_UNSIGNED_INT, nullptr);


        // glEnable(GL_DEPTH_TEST);
    }
};


