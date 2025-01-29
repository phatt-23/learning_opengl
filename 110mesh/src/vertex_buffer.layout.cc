//
// Created by phatt on 1/24/25.
//
module;

#include <GL/glew.h>
#include "std.h"

export module vertex_buffer.layout;

import vertex_buffer.supported_types;

export struct VertexBufferAttribute {
    // These properties are strictly read only.
    const GLuint dataTypeMacroCode; // What data type is this single attribute composed of.
    const GLuint count; // How many variables of specified `dataType` are there that describe a single attribute.
    const GLboolean normalized; // Matters only for integer types, usually just put in GL_FALSE.
    const std::string name; // If the name is specified, it specifies/comments on what the attribute means or what it's called.

    explicit VertexBufferAttribute(const GLuint dataTypeMacroCode, const GLuint count, const GLuint normalized, std::string name = "")
    : dataTypeMacroCode(dataTypeMacroCode), count(count), normalized(normalized), name(std::move(name)) {
        if (getSizeOfGLTypeFromMacroCode(dataTypeMacroCode) == -1) {
            throw std::runtime_error("Vertex buffer attribute '" + name
                + "' creation failed because of unsupported data type macro code '"
                + std::to_string(dataTypeMacroCode) + "'.");
        }
    }
};

export class VertexBufferLayout {
    // Stores attribute types and their count. The sum of the bytes used is stored in the `stride`.
    std::vector<VertexBufferAttribute> attributes{};
    // States how many bytes does one vertex take up using this layout.
    GLsizei stride = 0;
public:
    VertexBufferLayout() = default;
    ~VertexBufferLayout() = default;

    [[nodiscard]] auto getAttributes() const -> const std::vector<VertexBufferAttribute>& {
        return attributes;
    }

    [[nodiscard]] auto getStride() const -> GLsizei {
        return stride;
    }

    /// Adds new attribute of specified type and how many of them there is.
    /// You can check which data types are supported at the moment.
    /// You can also specify `attributeName` which is useful for debugging.
    /// The `attributeName` can basically comment on what the attribute means.
    /// It's better than just comments because it is part of the code itself.
    /// If it is not specified, it will by default be assigned empty string.
    template<typename DataType>
    requires IsSupportedByVertexBuffer<DataType> // this is redundant but whatever
    auto pushAttribute(const std::uint32_t count, const std::string& attributeName = "") -> VertexBufferLayout& {
        const GLuint dataTypeMacroCode = getGLTypeMacroCode<DataType>();
        const auto attribute = VertexBufferAttribute(dataTypeMacroCode, count, GL_FALSE, attributeName);
        attributes.push_back(attribute);
        stride += sizeof(DataType) * count;
        return *this;
    }

    /// Configures every attribute in the layout.
    /// Make sure to call binds of VAO and VBO properly before calling this function.
    auto configure() const -> void {
        // Prepare the offset accumulator.
        std::uint32_t offset = 0;
        for (int index = 0; index < attributes.size(); ++index) {
            const auto& attribute = attributes[index];
            // Enable configuring of the attribute variable at the index-th location.
            glEnableVertexAttribArray(index);
            // Configure the attribute variable at the index-th location.
            glVertexAttribPointer(index,
                static_cast<GLsizei>(attribute.count),
                attribute.dataTypeMacroCode,
                attribute.normalized,
                stride,
                reinterpret_cast<const void *>(offset));
            // Move the offset by the size of the attribute in bytes.
            offset += attribute.count * getSizeOfGLTypeFromMacroCode(attribute.dataTypeMacroCode);
        }
    }
};
