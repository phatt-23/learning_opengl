//
// Created by phatt on 1/30/25.
//
module;

#include "std.h"
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vector_relational.hpp>

export module transformation;

import shader_program;

export class Transformation {
private:
    glm::vec3 scaleVec; 
    glm::vec3 translationVec;
    glm::vec3 rotationAxis;
    float rotationInRadians;

    glm::mat4 scaleMat{};
    glm::mat4 rotationMat{};
    glm::mat4 translationMat{};

    glm::mat4 modelMat{};
public:
    explicit Transformation(
        const glm::vec3& translationVec = glm::vec3(0.f, 0.f, 0.f),
        const glm::vec3& rotationAxis = glm::vec3(0.f, 1.f, 0.f),
        const float rotationAmountInDegrees = 0.f,
        const glm::vec3& scaleVec = glm::vec3(1.f, 1.f, 1.f)) 
    : scaleVec(scaleVec), 
    rotationAxis(glm::normalize(rotationAxis)), 
    translationVec(translationVec),
    rotationInRadians(glm::radians(rotationAmountInDegrees)) {
        if (glm::length(this->rotationAxis) == 0.0) {
            std::println("Rotation axis must not have length of zero");
            this->rotationAxis = {0.f, 1.f, 0.f};
        }

        if (glm::all(glm::lessThanEqual(this->scaleVec, glm::vec3(0.f, 0.f, 0.f)))) {
            std::println("Scale vector must not have any axis less than or equal to zero");
            this->scaleVec = {0.f, 1.f, 0.f};
        }

        scaleMat = glm::scale(glm::mat4(1.f), this->scaleVec);
        rotationMat = glm::rotate(glm::mat4(1.f), this->rotationInRadians, this->rotationAxis);
        translationMat = glm::translate(glm::mat4(1.f), this->translationVec);
        modelMat = translationMat * rotationMat * scaleMat;
    }
    
    [[nodiscard]] auto getModelMat() const -> const glm::mat4& {
        return modelMat;
    }

    auto sendModelMatToShader(
        ShaderProgram& shader, 
        const std::string& uniformModelMatName
    ) const -> void {
        shader.bind();
        shader.setUniformMat4f(uniformModelMatName, getModelMat());
        ShaderProgram::unbind();
    }
};

