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
    glm::f32vec3 color;
    glm::f32vec2 texUV;

    static auto getLayout() -> VertexBufferLayout {
        return VertexBufferLayout()
            .pushAttribute<glm::f32>(3, "Position")
            .pushAttribute<glm::f32>(3, "Normal")
            .pushAttribute<glm::f32>(3, "Color")
            .pushAttribute<glm::f32>(2, "TexUV");
    }

    static auto create(
        glm::f32vec3 position,
        glm::f32vec3 normal,
        glm::f32vec3 color,
        glm::f32vec2 texUV
    ) -> Vertex {
        return Vertex{ .position=position, .normal=normal, .color=color, .texUV=texUV };
    }

    // Vertex(
    //     glm::f32vec3 position,
    //     glm::f32vec3 normal,
    //     glm::f32vec3 color,
    //     glm::f32vec2 texUV) 
    // : position(std::move(position)), normal(std::move(normal))
    // , color(std::move(color)), texUV(std::move(texUV)) {}
};

