//
// Created by phatt on 1/26/25.
//
#include <unordered_map>
module;

#include "std.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module camera;

import mouse;
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

export namespace camera::defaults {
    constexpr auto worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    constexpr float near = .1f;
    constexpr float far = 100.f;
    constexpr float fov = 45.f;
    constexpr float movementSpeed = 3.f;
    constexpr auto front = glm::vec3(0.0f, 0.0f, -1.0f);
    constexpr auto up = glm::vec3(0.0f, 1.0f, 0.0f);
    constexpr auto right = glm::vec3(1.0f, 0.0f, 0.0f);
    constexpr auto position = glm::vec3(0.0f, 0.0f, 4.0f);
    constexpr float yaw = -90.f;
    constexpr float pitch = 0.f;
}

/// TODO: Should be a singleton class so make it one.
///
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
    float fov = camera::defaults::fov;
    // The camera's near and far plane.
    float near = camera::defaults::near;
    float far = camera::defaults::far;

    // The camera can change its `position` vector. To do this we usually use key presses.
    // We want to be able to control the camera's movement speed to it doesn't fly around too fast or too slow.
    float movementSpeed = camera::defaults::movementSpeed;

    // Where the camera is placed in the world coordinates.
    glm::vec3 position = camera::defaults::position;
    // The direction the camera points at/looks at.
    glm::vec3 front = camera::defaults::front;
    // The up direction of the camera. It is used when the camera rotates around the 'roll' axis.
    // It is a cross vector of right direction and front direction.
    glm::vec3 up = camera::defaults::up;
    // Where is right direction. Is used to calculate the up direction.
    // It itself is a cross vector of front direction and world up direction.
    // Because it depends on world's up direction and not camera's up direction
    // it will always be parallel to the xy-plane.
    glm::vec3 right = camera::defaults::right;
    // The `forward` vector isn't the same as `front`, it is always parallel to the xy-plane.
    // Whereas `front` depends on the **camera's up** vector, the `forward` vector depends on the **world's up** direction.
    glm::vec3 forward = glm::cross(camera::defaults::worldUp, camera::defaults::right);

    // The camera's rotation angles around its own three axis.
    // We don't need the roll angle, because it can be calculated
    // from camera's up direction and world coordinate's up direction.
    float yaw = camera::defaults::yaw;
    float pitch = camera::defaults::pitch;

    // Store for the calculated projection-view matrix of the camera.
    glm::mat4 projectionViewMatrix = glm::mat4(1.0f);
public:
    /// Pure constructor. <br>
    /// Must provide `displayDimensions`. (crucial) <br>
    /// You should provide `movementSpeed`, `position`, `front`, `up` (by default the same as `worldUp`). (very useful) <br>
    /// You probably want to provide `fov`, `near` and `far` plane - which modify the viewing frustum. (useful settings) <br>
    /// You probably don't ever want to change the `yaw` or the `pitch` by yourself (it is quite useless). <br>
    explicit Camera(
        const glm::i32vec2& displayDimensions,
        float movementSpeed = camera::defaults::movementSpeed,
        const glm::vec3& position = camera::defaults::position,
        const glm::vec3& front = camera::defaults::front,
        const glm::vec3& up = camera::defaults::up,
        float fov = camera::defaults::fov,
        float near = camera::defaults::near,
        float far = camera::defaults::far,
        float yaw = camera::defaults::yaw,
        float pitch = camera::defaults::pitch)
    : displayDimensions(displayDimensions), aspectRatio(static_cast<float>(displayDimensions.x) / static_cast<float>(displayDimensions.y))
    , fov(fov), near(near), far(far)
    , movementSpeed(movementSpeed), position(position)
    , front(front), up(up), yaw(yaw), pitch(pitch) {
        updateOrientation();
    }

    /// Constructor depending on GLFW window to calculate the camera's `displayDimensions` and `aspectRatio`. <br>
    /// Must provide GLFWwindow pointer `window` - needed for display dimensions and aspect ratio. (crucial) <br>
    /// You should provide `movementSpeed`, `position`, `front`, `up` (by default the same as `worldUp`). (very useful) <br>
    /// You probably want to provide `fov`, `near` and `far` plane - which modify the viewing frustum. (useful settings) <br>
    /// You probably don't ever want to change the `yaw` or the `pitch` by yourself (it is quite useless). <br>
    explicit Camera(
        GLFWwindow* window,
        float movementSpeed = camera::defaults::movementSpeed,
        const glm::vec3& position = camera::defaults::position,
        const glm::vec3& front = camera::defaults::front,
        const glm::vec3& up = camera::defaults::up,
        float fov = camera::defaults::fov,
        float near = camera::defaults::near,
        float far = camera::defaults::far,
        float yaw = camera::defaults::yaw,
        float pitch = camera::defaults::pitch)
    : displayDimensions(0), fov(fov), near(near), far(far), movementSpeed(movementSpeed), position(position)
    , front(front), up(up), yaw(yaw), pitch(pitch) {
        glfwGetFramebufferSize(window, &displayDimensions.x, &displayDimensions.y);
        aspectRatio = static_cast<float>(displayDimensions.x) / static_cast<float>(displayDimensions.y);
        updateOrientation();
    }

    ~Camera() = default;

    /// The view matrix says where the camera is positioned the world coordinates
    /// and where it points/looks at. Affected by the camera's position and front and up facing vectors.
    [[nodiscard]] inline auto getViewMatrix() const -> glm::mat4 {
        return glm::lookAt(position, position + front, up);
    }

    /// The projection matrix says how the camera views. How the camera's 'objective' works.
    /// What shape will the viewing frustum take. Affected by the FOV, aspect ratio and near/far planes.
    [[nodiscard]] inline auto getProjectionMatrix() const -> glm::mat4 {
        return glm::perspective(glm::radians(fov), aspectRatio, near, far);
    }

    /// Updates the camera's transformation matrix.
    [[maybe_unused]] inline auto updateProjectionViewMatrix() -> const glm::mat4& {
        return (projectionViewMatrix = getProjectionMatrix() * getViewMatrix());
    }

    /// The camera's transformation matrix: projection matrix * view matrix .
    /// When applied to world coordinate system, the world will move and rotate around the camera
    /// making the camera seemingly move in the world and change its viewing direction.
    [[nodiscard]] inline auto getProjectionViewMatrix() const -> const glm::mat4& {
        return projectionViewMatrix;
    }

    /// Sends the camera's (proj * view) matrix to the shader program's
    /// uniform variable with name specified by `variableName`.
    /// NOTE: this function binds the shader, sends the camera's matrix to the GPU shader code and unbinds the shader.
    auto sendProjectionViewMatToShader(
        ShaderProgram& shader, 
        const std::string& uniformVariableName
    ) const -> void {
        shader.bind();
        shader.setUniformMat4f(uniformVariableName, getProjectionViewMatrix());
        ShaderProgram::unbind();
    }

    /// Sends the camera's position vec3 to the GPU shader code's uniform vec3
    /// with the name of `uniformVariableName`
    auto sendPositionToShader(
        ShaderProgram& shader, 
        const std::string& uniformVariableName
    ) const -> void {
        shader.bind();
        shader.setUniform3f(uniformVariableName, this->position);
        ShaderProgram::unbind();
    }

    /// Sets camera's display dimensions.
    auto setDisplayDimensions(const glm::i32vec2& displayDimensions) -> void {
        this->displayDimensions = displayDimensions;
    }

    /// Update camera's orientation vectors (front, right, up) based
    /// on the updated `yaw` and `pitch` angles.
    /// NOTE: Call this after updating the `yaw` and the `pitch` angles.
    auto updateOrientation() -> void {
        // Clamp the pitch angle [-90, 90] and wrap the yaw angle [-180, 180].
        yaw = wrap(yaw, -180.f, 180.f);
        pitch = clamp(pitch, -89.f, 89.f);
        // Front facing vector depends on yaw and pitch.
        front = glm::normalize(
            glm::vec3(
                std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
                std::sin(glm::radians(pitch)),
                std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch))
            ));
        // Right direction is always parallel to the xy-plane because we use world's up direction.
        // NOTE: You can get the cross vectors x and y (cross(x, y)) by using the right hand rule.
        //       Index finger points in x direction, middle in y direction and thumb sticks in the
        //       direction of the cross product of x and y.
        right = glm::normalize(glm::cross(front, camera::defaults::worldUp));
        // The camera's up direction is always orthogonal to the xy-plane.
        // The camera doesn't rotate in the roll axis.
        up = glm::normalize(glm::cross(right, front));
        forward = glm::normalize(glm::cross(camera::defaults::worldUp, right));
    }

    /// Handles keyboard events, mouse events and updates internal camera variables.
    /// Updates the camera's orientation angles - yaw, pitch.
    /// Updates the camera's position.
    /// Updates camera projection-view matrix.
    /// Should be called in the main loop on every frame.
    auto onNextFrame(GLFWwindow *window, const double deltaTime) -> void {
        // Process user input 
        // that update camera position
        processKeyboardInput(window, deltaTime);
        // and changes the yaw and pitch angles.
        processMouseInput(window, deltaTime);
        
        // Apply these changes in yaw and pitch angles 
        // to update the orientation vectors.
        updateOrientation();
        // Update display dimension and aspect ratio.
        glfwGetFramebufferSize(window, &displayDimensions.x, &displayDimensions.y);
        aspectRatio = static_cast<float>(displayDimensions.x) / static_cast<float>(displayDimensions.y);
        // Update the camera's projection-view matrix after all 
        // orientation vectors and frame dimensions were updated. 
        updateProjectionViewMatrix();
        
        // std::cout << "camera pos: " << position.x << ", " << position.y << ", " << position.z << std::endl;
        // std::cout << "camera front: " << front.x << ", " << front.y << ", " << front.z << std::endl;
        // std::cout << "camera yaw pitch: " << yaw << ", " << pitch << std::endl;
    }

    /// Processes user keyboard input and updates the camera's position and speed.
    /// Key presses update camera's position.
    auto processKeyboardInput(GLFWwindow* window, const double deltaTime) -> void {
        const float speedMultiplier = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 3.f : 1.f;
        const float speed = movementSpeed * speedMultiplier * static_cast<float>(deltaTime);
        
        static const auto keyBindPressActions = std::unordered_map<std::uint32_t, std::function<void()>> {
            {GLFW_KEY_W,            [&]() { position += forward * speed; }},
            {GLFW_KEY_A,            [&]() { position -= right * speed; }},
            {GLFW_KEY_S,            [&]() { position -= forward * speed; }},
            {GLFW_KEY_D,            [&]() { position += right * speed; }},
            {GLFW_KEY_SPACE,        [&]() { position += camera::defaults::worldUp * speed; }},
            {GLFW_KEY_LEFT_CONTROL, [&]() { position -= camera::defaults::worldUp * speed; }},
        };
        
        for (const auto& [keyBind, pressAction] : keyBindPressActions) {
            if (glfwGetKey(window, keyBind) == GLFW_PRESS) {
                pressAction();
            }
        }
    }

    /// Processes user mouse input and update the camera's `yaw` and `pitch` orientation.
    /// Mouse movement and click update camera's yaw and pitch.
    auto processMouseInput(GLFWwindow* window, const double deltaTime) -> void {
        // std::printf( "Mouse modes: %x\n", Mouse::getInstance().getOperationalModes() );

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            Mouse::getInstance().enableMode(mouse::mode::is_sensing_movement);
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            Mouse::getInstance().disableMode(mouse::mode::is_sensing_movement);
        }

        if (Mouse::getInstance().inMode(mouse::mode::is_sensing_movement)) {
            const auto cursor = glm::f32vec2(Mouse::getInstance().getCursorPosition());
            const auto lastCursor = glm::f32vec2(Mouse::getInstance().getLastCursorPosition());
            const auto sensitivity = static_cast<float>(Mouse::getInstance().getMovementSensitivity());

            yaw += (cursor.x - lastCursor.x) * sensitivity;
            pitch -= (cursor.y - lastCursor.y) * sensitivity;
        }

    }
};

