//
// Created by phatt on 1/24/25.
//
module;

#include <GL/glew.h>
#include "std.h"
#include "stb_image.h"

export module texture;

// Wrapper class over the OpenGL texture object.
export class Texture {
private:
    const std::string filepath; // The filepath to the texture image. Useful for debugging.
    GLuint textureID = 0; // The reference ID to the OpenGL texture object.
    GLenum textureType = GL_TEXTURE_2D; // Can 1D, 2D or 3D. Most common is GL_TEXTURE_2D.
    GLenum dataFormat = GL_RGBA; // If it's JPG, use GL_RGB, if it's PNG use GL_RGBA.
    int width = 0; // Width of the texture image in pixels.
    int height = 0; // Height of the texture image in pixels.
    int channels = 0; // Bits per pixel. Usually 3 or 4.
public:
    /// CAUTION: Expects the texture unit slot to be a regular number not OpenGL macro.
    explicit Texture(
        std::string filepath,
        const GLenum textureType = GL_TEXTURE_2D,
        const GLuint textureUnitSlot = 0
        )
    : filepath(std::move(filepath)) {
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

        // Based on the number of channels get the OpenGL format
        // by which we will transfer the data from CPU to GPU.
        GLint internalFormat = 0; // This internal format should be the same as the `dataFormat` member variable.
        switch (channels) {
            case 1:
                internalFormat = GL_RED;
                dataFormat = GL_RED;
            break;
            case 3:
                internalFormat = GL_RGB;
                dataFormat = GL_RGB;
            break;
            case 4:
                internalFormat = GL_RGBA;
                dataFormat = GL_RGBA;
            break;
            default:
                throw std::runtime_error(std::format("Error while loading '{}' because of unsupported number of channels: {}", this->filepath, channels));
        }

        // Generate the texture object ID.
        glGenTextures(1, &textureID);
        // Activating texture unit at specified slot and binding the texture to generated it.
        // Activate the slot where we will put the created texture object.
        glActiveTexture(GL_TEXTURE0 + textureUnitSlot);
        // Putting it into the specified slot.
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Setting texture parameters.
        // Configure how will the image behave when it becomes smaller and bigger.
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Configure the way texture repeats if at all.
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // You can also specify the border color if the GL_TEXTURE_WRAP_S/T are set to be bordered.
        constexpr float flatColor[4] = { 1.f, 1.f, 1.f, 1.f };
        glTextureParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

        // Transfer the image data from CPU to the GPU.
        // The color channel is different for PNG and JPG images (JPG doesn't have alpha channel)
        // For PNG images use GL_RGBA and for JPG images use GL_RGB as `internalformat` and `format`
        glTexImage2D(textureType, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, imageBytes);
        // Free the image data on the CPU side as it is already been transferred.
        stbi_image_free(imageBytes);

        // Generates the mip maps of the same picture, which are smaller versions of the same image.
        // E.g. The smaller mip map will be used if the image is far away.
        glGenerateMipmap(GL_TEXTURE_2D);

        // Unbind the texture unit slot, just in case.
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~Texture() {
        // Frees the texture object with reference ID of `textureID` from the GPU.
        glDeleteTextures(1, &textureID);
    }

    /// Binds texture to specified texture unit slot.
    /// CAUTION: Expects the texture unit slot to be a regular number not OpenGL macro.
    auto bind(const uint textureUnitSlot = 0) const -> void {
        glActiveTexture(GL_TEXTURE0 + textureUnitSlot);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    /// Unbinds texture from the texture unit slots.
    /// There is no need to free the slots themselves,
    /// they will just be replaced by other texture objects.
    static auto unbind() -> void {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

