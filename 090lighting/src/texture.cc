//
// Created by phatt on 1/24/25.
//
module;

#include "std.h"
#include <GL/glew.h>
#include "stb_image.h"

export module texture;

import shader_program;

export namespace texture {
    enum class Type : GLenum {
        $1D = GL_TEXTURE_1D,
        $2D = GL_TEXTURE_2D,
        $3D = GL_TEXTURE_3D,
    };

    enum class DataFormat : GLenum {
        NotSpecified = 0,
        R = GL_RED,
        RG = GL_RG,
        RGB = GL_RGB,
        RGBA = GL_RGBA,
        DepthComponent = GL_DEPTH_COMPONENT,
        DepthStencil = GL_DEPTH_STENCIL,
    };

    std::string textureTypeToString(const Type type) {
        switch (type) {
            case Type::$1D: return "1D"; break;
            case Type::$2D: return "2D"; break;
            case Type::$3D: return "3D"; break;
            default: return "unknown"; break;
        }
    }

    auto textureDataFormatToString(const DataFormat dataFormat) -> std::string {
        switch (dataFormat) {
            case DataFormat::NotSpecified: return "notspecified"; break;
            case DataFormat::R: return "r"; break;
            case DataFormat::RG: return "RG"; break;
            case DataFormat::RGB: return "RGB"; break;
            case DataFormat::RGBA: return "RGBA"; break;
            case DataFormat::DepthComponent: return "depthcomponent"; break;
            case DataFormat::DepthStencil: return "depthstencil"; break;
            default: return "unknown"; break;
        }
    }
}

using namespace texture;

/// Wrapper class over the OpenGL texture object.
/// This class does not hold information about which slot to put the texture object into.
/// When you want to send the texture object to the GPU shader code you must specify in
/// which slot it goes to.
export class Texture {
private:
    const std::string filepath; // The filepath to the texture image. Useful for debugging.
    GLuint textureID = 0; // The reference ID to the OpenGL texture object.
    Type textureType; // Can 1D, 2D or 3D. Most common is GL_TEXTURE_2D.
    DataFormat dataFormat; // If it's JPG, use GL_RGB, if it's PNG use GL_RGBA.
    int width = 0; // Width of the texture image in pixels.
    int height = 0; // Height of the texture image in pixels.
    int channels = 0; // Bits per pixel. Usually 3 or 4.
    int lastTextureUnitSlotIndex = 0;  // For correct unbinding of the texture objects.
public:
    /// Note that the `textureUnitSlot` is used only for the texture initialisation.
    /// It is not stored in this class at all. On the other hand, the `textureType` is
    /// being stored inside this class and is used in binding/unbinding functions.
    /// CAUTION: Expects the texture unit slot to be a regular number not OpenGL macro.
    ///          The `dataFormat` must be specified when using depth or depth-stencil textures.
    explicit Texture(
        const std::string& filepath,
        const Type type,
        const DataFormat format = DataFormat::NotSpecified,
        const int textureUnitSlot = 0
        )
    : filepath(filepath), textureType(type), dataFormat(format)
    , lastTextureUnitSlotIndex(textureUnitSlot) {
        printf("Loading texture %s\n", this->filepath.c_str());
        // Creating OpenGL texture object and loading the texture to the CPU.
        // OpenGL read the data from left to right, bottom up while STB lib reads left to right, top to bottom.
        // Flipping the default direction is necessary.
        stbi_set_flip_vertically_on_load(true);

        // Load the image data with the help of the STB library and let the function set out width, height and channels.
        stbi_uc *imageBytes = stbi_load(this->filepath.c_str(), &width, &height, &channels, 0);
        // Check if the load was successful.
        if (imageBytes == nullptr) {
            throw std::runtime_error("Failed to load texture: " + filepath + "\n");
        }

        // If the dataFormat was not specified make an assumption
        // that the texture is of formats R, RG, RGB or RGBA
        // based on the number of channels get the OpenGL format
        // by which we will transfer the data from CPU to GPU.
        if (dataFormat == DataFormat::NotSpecified) {
            switch (channels) {
                case 1: { dataFormat = DataFormat::R; } break;
                case 2: { dataFormat = DataFormat::RG; } break;
                case 3: { dataFormat = DataFormat::RGB; } break;
                case 4: { dataFormat = DataFormat::RGBA; } break;
                default: { throw std::runtime_error(
                        std::format("Error while loading '{}' because of unsupported number of channels: {}",
                            this->filepath, channels));
                }
            }
        }

        // Generate the texture object ID.
        glGenTextures(1, &textureID);
        // Activating texture unit at specified slot and binding the texture to generated it.
        // Activate the slot where we will put the created texture object.
        glActiveTexture(GL_TEXTURE0 + textureUnitSlot);
        // Putting it into the specified slot.
        glBindTexture(static_cast<GLenum>(textureType), textureID);

        // Transfer the image data from CPU to the GPU.
        // The color channel is different for PNG and JPG images (JPG doesn't have alpha channel)
        // For PNG images use GL_RGBA and for JPG images use GL_RGB as `internalformat` and `format`
        glTexImage2D(static_cast<GLenum>(textureType),
            0,
            static_cast<GLint>(dataFormat),
            width,
            height,
            0,
            static_cast<GLenum>(dataFormat),
            GL_UNSIGNED_BYTE,
            imageBytes);
        // Free the image data on the CPU side as it is already been transferred.
        stbi_image_free(imageBytes);

        // Generates the mip maps of the same picture, which are smaller versions of the same image.
        // E.g. The smaller mip map will be used if the image is far away.
        glGenerateMipmap(static_cast<GLenum>(textureType));

        // Setting texture parameters.
        // Configure the way texture repeats if at all.
        glTextureParameteri(static_cast<GLuint>(textureType), GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(static_cast<GLuint>(textureType), GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Configure how will the image behave when it becomes smaller and bigger.
        glTextureParameteri(static_cast<GLuint>(textureType), GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTextureParameteri(static_cast<GLuint>(textureType), GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // You can also specify the border color if the GL_TEXTURE_WRAP_S/T are set to be bordered.
        constexpr float flatColor[4] = { 1.f, 1.f, 1.f, 1.f };
        glTextureParameterfv(static_cast<GLuint>(textureType), GL_TEXTURE_BORDER_COLOR, flatColor);

        // Unbind the texture unit slot, just in case.
        glBindTexture(static_cast<GLenum>(textureType), 0);
    }

    ~Texture() {
        // Frees the texture object with reference ID of `textureID` from the GPU.
        glDeleteTextures(1, &textureID);
    }

    /// Binds texture object to specified texture unit slot.
    /// CAUTION: Expects the texture unit slot to be a regular number and not the OpenGL macro.
    auto bind(const GLint textureUnitSlotIndex) -> void {
        glActiveTexture(GL_TEXTURE0 + textureUnitSlotIndex);
        glBindTexture(static_cast<GLenum>(textureType), textureID);
        lastTextureUnitSlotIndex = textureUnitSlotIndex;
    }

    auto bind() const -> void {
        glActiveTexture(GL_TEXTURE0 + lastTextureUnitSlotIndex);
        glBindTexture(static_cast<GLenum>(textureType), textureID);
    }

    /// Unbinds texture object from the texture unit slot
    /// that it was last time bound into.
    /// CAUTION: It will unbind whatever texture object is in the slot
    /// where this texture object was last time put into.
    /// NOTE: Calling this is not really needed.
    auto unbind() const -> void {
        glActiveTexture(GL_TEXTURE0 + lastTextureUnitSlotIndex);
        glBindTexture(static_cast<GLenum>(textureType), 0);
    }

    /// TODO: This is not correct. This function can be static.
    /// TODO: It only needs to set the uniform sampler variables
    /// TODO: in the shader code.
    /// TODO:    GL_TEXTURE0 -> uniform sampler2D U_Texture0 = 0
    /// TODO:    GL_TEXTURE1 -> uniform sampler2D U_Texture1 = 1
    /// TODO: And then the texture objects can assign themselves
    /// TODO: to the texture unit slots.
    /// TODO: The shader code and the texture unit slots are NOT coupled.
    ///
    /// Send the texture object this class holds to the shader code's
    /// uniform sampler variable with name specified by `uniformSamplerVariableName`.
    /// It specifically sets the uniform variable to be the index of a texture unit slot
    /// where the texture object is bounded in.
    ///
    /// I'm planning on standardising the way I name uniform variables
    /// in the shader code, so I can just simply pass in the slot index.
    /// For now the `uniformSamplerVariableName` should be assigned
    /// with "U_Texture(i)" where (i) is the index of a texture unit slot.
    /// The `textureUnitSlotIndex` should match this (i) number.
    // auto sendTextureToShader(
    //
    /// If we are using GL_TEXTURE0 we assign '0' to the sampler2D variable
    /// representing the texture unit slot at 0-th index in the shader code.
    /// If we use GL_TEXTURE4 we assign 4 and so on. In this case
    /// `textureUnitSlot` is provided by the caller.
    static auto setTextureSamplerInShader(
        ShaderProgram& shader,
        const std::string& uniformSamplerVariableName,
        const GLint textureUnitSlotIndex
    ) -> void {
        // Make sure to bind the shader program first.
        // Bind the shader to send the texture object to the texture unit
        shader.bind();
        shader.setUniform1i(uniformSamplerVariableName, textureUnitSlotIndex);
        ShaderProgram::unbind();
    }
};

