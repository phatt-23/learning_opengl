//
// Created by phatt on 1/26/25.
//
module;

#include "std.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module camera;

import shader_program;

template<typename T> concept IsNumeric = std::is_integral_v<T> || std::is_floating_point_v<T>;

template<IsNumeric T>
auto clamp(T value, T low, T high) -> T {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

template<IsNumeric T>
auto wrap(T value, T low, T high) -> T {
    if (value < low) return high;
    if (value > high) return low;
    return value;
}

export namespace camera_default {
    constexpr auto worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    constexpr float near = .1f;
    constexpr float far = 100.f;
    constexpr float fov = 45.f;
    constexpr float movementSpeed = 10.f;
    constexpr auto front = glm::vec3(0.0f, 0.0f, -1.0f);
    constexpr auto up = glm::vec3(0.0f, 1.0f, 0.0f);
    constexpr auto right = glm::vec3(1.0f, 0.0f, 0.0f);
    constexpr auto position = glm::vec3(0.0f, 0.0f, 3.0f);
    constexpr float yaw = -90.f;
    constexpr float pitch = 0.f;
}

/// Camera can be thought of as an object that wraps around a single `front` vector in 3-dimensional space.
/// This `front` vector says where the camera points to and is normalized, meaning its distance from the origin is one.
/// So it's 'trapped' inside this sphere with its origin being the `position` of the camera
/// and this `front` vector can change its direction but never leave this sphere's surface.
///
/// The camera can rotate in three directions - yaw (left to right), pitch (bottom up), roll (side to side, like arms of a clock)
/// Note that roll angle is not needed because we already have world up direction and camera's up direction.
///
/// The `position` of the camera is changed usually by the keyboard events - WASD, arrow, etc.
/// While the `front` vector is changed usually by mouse movements which is actually changing the yaw and pitch of the camera.
///
/// To calculate the `front` vector we depend on:
///  - camera's pitch angle
///  - camera's yaw angle
///
/// To calculate the camera's `right` vector direction we need:
///  - world coordinate's up direction
///  - camera's front direction
///
/// To calculate the camera's `up` vector direction we need:
///  - camera's front direction
///  - camera's right direction
///
/// Camera can see objects only if they are in the view frustum.
/// This view frustum is parametrised by the camera's `field of view`, `near` plane and `far` plane.
/// It also depends on the `aspect ratio` that in turn depends on window screen dimensions.
export class Camera {
private:
    // The world coordinate's up direction. Should stay constant.
    // If it changes, that means the world is rotating around 'roll' or 'pitch' axis.
    // We probably don't ever want that.
    // The screen window's dimensions to calculate the screen's aspect ratio.
    glm::i32vec2 displayDimensions;
    // Aspect ratio is needed so the projected image on the screen of the window
    // isn't warped if the window isn't sized 1:1.
    float aspectRatio;
    // The camera's field of view.
    float fov = camera_default::fov;
    // The camera's near and far plane.
    float near = camera_default::near;
    float far = camera_default::far;

    // The camera can change its `position` vector. To do this we usually use key presses.
    // We want to be able to control the camera's movement speed to it doesn't fly around too fast or too slow.
    float movementSpeed =camera_default::movementSpeed;

    // Where the camera is placed in the world coordinates.
    glm::vec3 position = camera_default::position;
    // The direction the camera points at/looks at.
    glm::vec3 front = camera_default::front;
    // The up direction of the camera. It is used when the camera rotates around the 'roll' axis.
    // It is a cross vector of right direction and front direction.
    glm::vec3 up = camera_default::up;
    // Where is right direction. Is used to calculate the up direction.
    // It itself is a cross vector of front direction and world up direction.
    // Because it depends on world's up direction and not camera's up direction
    // it will always be parallel to the xy-plane.
    glm::vec3 right = camera_default::right;

    // The camera's rotation angles around its own three axis.
    // We don't need the roll angle, because it can be calculated
    // from camera's up direction and world coordinate's up direction.
    float yaw = camera_default::yaw;
    float pitch = camera_default::pitch;
public:
    /// Pure constructor.
    explicit Camera(
        const glm::i32vec2& displayDimensions,
        float movementSpeed = camera_default::movementSpeed,
        const glm::vec3& position = camera_default::position,
        const glm::vec3& front = camera_default::front,
        const glm::vec3& up = camera_default::up,
        float fov = camera_default::fov,
        float yaw = camera_default::yaw,
        float pitch = camera_default::pitch,
        float near = camera_default::near,
        float far = camera_default::far)
    : displayDimensions(displayDimensions), aspectRatio(displayDimensions.x / displayDimensions.y)
    , fov(fov), near(near), far(far)
    , movementSpeed(movementSpeed), position(position)
    , front(front), up(up), yaw(yaw), pitch(pitch) {}

    /// Constructor depending on GLFW window to calculate the camera's displayDimensions and aspectRatio.
    explicit Camera(
        GLFWwindow* window,
        float movementSpeed = camera_default::movementSpeed,
        const glm::vec3& position = camera_default::position,
        const glm::vec3& front = camera_default::front,
        const glm::vec3& up = camera_default::up,
        float fov = camera_default::fov,
        float yaw = camera_default::yaw,
        float pitch = camera_default::pitch,
        float near = camera_default::near,
        float far = camera_default::far)
    : fov(fov), near(near), far(far), movementSpeed(movementSpeed), position(position)
    , front(front), up(up), yaw(yaw), pitch(pitch) {
        glfwGetFramebufferSize(window, &displayDimensions.x, &displayDimensions.y);
        aspectRatio = displayDimensions.x / displayDimensions.y;
    }

    ~Camera() = default;

    /// The view matrix says where the camera is positioned the world coordinates
    /// and where it points/looks at. Affected by the camera's position and front and up facing vectors.
    inline auto getViewMatrix() const -> glm::mat4 {
        return glm::lookAt(position, position + front, up);
    }

    /// The projection matrix says how the camera views. How the camera's 'objective' works.
    /// What shape will the viewing frustum take. Affected by the FOV, aspect ratio and near/far planes.
    inline auto getProjectionMatrix() const -> glm::mat4 {
        return glm::perspective(glm::radians(fov), aspectRatio, near, far);
    }

    /// The camera's transformation matrix: view matrix multiplied by projection matrix.
    /// When applied to world coordinate system, the world will move and rotate around the camera
    /// making the camera seemingly move in the world and change its viewing direction.
    inline auto getViewProjectionMatrix() const -> glm::mat4 {
        return getViewMatrix() * getProjectionMatrix();
    }

    /// Sends the camera's (view * proj) matrix to the shader program's
    /// uniform variable with name specified by `variableName`.
    /// NOTE: this function binds the shader, sends the camera's matrix to the GPU shader code and unbinds the shader.
    auto sendViewProjectionMatToShader(ShaderProgram& shader, const std::string& uniformVariableName) const -> void {
        shader.bind();
        shader.setUniformMat4f(uniformVariableName, getViewProjectionMatrix());
        shader.unbind();
    }

    /// Sets camera's display dimensions.
    auto setDisplayDimensions(const glm::i32vec2& displayDimensions) -> void {
        this->displayDimensions = displayDimensions;
    }

    auto onUpdate() -> void {
        // Update camera's orientation.
        // Clamp the pitch angle [-90, 90] and wrap the yaw angle [-180, 180].
        yaw = wrap(yaw, -180.f, 180.f);
        pitch = wrap(pitch, -90.f, 90.f);

        // Front facing vector depends on yaw and pitch.
        front = glm::normalize(
            glm::vec3(
                std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
                std::sin(glm::radians(pitch)),
                std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch))
            ));

        // Right direction is always parallel to the xy-plane because we use world's up direction.
        right = glm::normalize(glm::cross(front, camera_default::worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};