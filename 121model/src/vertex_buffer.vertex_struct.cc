//
// Created by phatt on 1/28/25.
//

module;

#include "std.h"
#include <glm/glm.hpp>

export module vertex_buffer.vertex_struct;

import vertex_buffer.layout;

export struct Vertex {
    glm::f32vec3 position;
    glm::f32vec3 normal;
    glm::f32vec2 texUV;
    glm::f32vec3 tangent;
    glm::f32vec3 bitangent;

    [[nodiscard]] static auto getLayout() -> VertexBufferLayout {
        return VertexBufferLayout()
            .pushAttribute<glm::f32>(3, "Position")
            .pushAttribute<glm::f32>(3, "Normal")
            .pushAttribute<glm::f32>(2, "TexUV")
            .pushAttribute<glm::f32>(3, "Tangent")
            .pushAttribute<glm::f32>(3, "BiTangent");
    }

    [[nodiscard]] static auto create(
        const glm::f32vec3 position,
        const glm::f32vec3 normal,
        const glm::f32vec2 texUV,
        const glm::f32vec3 tangent,
        const glm::f32vec3 bitangent
    ) -> Vertex {
        return Vertex {
            .position=position,
            .normal=normal,
            .texUV=texUV,
            .tangent=tangent,
            .bitangent=bitangent
        };
    }
};

