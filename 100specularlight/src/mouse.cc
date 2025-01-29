//
// Created by phatt on 1/27/25.
//

module;

#include "std.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

export module mouse;

export namespace mouse::defaults {
    constexpr auto cursorPosition = glm::f64vec2(0.0f);
    constexpr auto lastCursorPosition = glm::f64vec2(0.0f);
    constexpr auto scrollOffset = glm::f64vec2(0.0f);
    constexpr double movementSensitivity = .2f;
    constexpr double scrollSensitivity = 1.f;
}

export namespace mouse::mode {
    /// Describes the mouse's operational mode.
    enum ModeEnum : std::uint8_t {
        is_sensing_movement     = 0b00000001,
        is_sensing_mouse_button = 0b00000010,
        is_sensing_scroll       = 0b00000100,
    };
}

/// TODO: Singleton class for mouse inputs like the
/// Mouse class providing inputs of the mouse like the
/// cursor position its last position and the mouse wheel scroll offset.
/// NOTE: I'm not implementing any other device than
/// an ordinary mouse with a wheel and a movement sensor.
export class Mouse {
private:
    /// The window it figures in.
    GLFWwindow* window;
    /// Enumeration telling in which operating mode this mouse is in.

    glm::f64vec2 cursorPosition = mouse::defaults::cursorPosition;
    glm::f64vec2 lastCursorPosition = mouse::defaults::lastCursorPosition;
    double movementSensitivity = mouse::defaults::movementSensitivity;

    glm::f64vec2 scrollOffset = mouse::defaults::cursorPosition;
    double scrollSensitivity = mouse::defaults::scrollSensitivity;

    static Mouse* instance;

    std::uint8_t operationalModes = 0b00000000;
private:
    explicit Mouse(
        GLFWwindow* window,
        const double movementSensitivity,
        const double scrollSensitivity,
        const std::uint8_t operationalModes)
    : window(window), movementSensitivity(movementSensitivity), scrollSensitivity(scrollSensitivity)
    , operationalModes(operationalModes) {
        glfwSetWindowUserPointer(window, this);

        glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xPos, double yPos) -> void {
            const auto self = static_cast<Mouse *>(glfwGetWindowUserPointer(win));
            if (self == nullptr) {
                return;
            }
            self->lastCursorPosition = self->cursorPosition;
            self->cursorPosition = glm::i32vec2(xPos, yPos);
        });

        glfwSetScrollCallback(window, [](GLFWwindow* win, double xOffset, double yOffset) -> void {
            const auto self = static_cast<Mouse *>(glfwGetWindowUserPointer(win));
            if (self == nullptr) {
                return;
            }
            self->scrollOffset = glm::f64vec2(xOffset, yOffset);
        });
    }

    ~Mouse() = default;
public:
    /// If the mouse singleton is not created yet then create it and return the instance.
    /// If it was already created, nothing happens, it just returns the mouse instance.
    /// You must call this before you can use the singleton instance.
    /// The instance cannot be created by itself. It must know in which
    /// GLFWwindow it works with.
    static auto createInstance(
        GLFWwindow* window,
        const float movementSensitivity = mouse::defaults::movementSensitivity,
        const float scrollSensitivity = mouse::defaults::scrollSensitivity,
        const std::uint8_t operationalModes = mouse::mode::is_sensing_mouse_button | mouse::mode::is_sensing_scroll
    ) -> Mouse& {
        if (instance == nullptr) {
            instance = new Mouse(window, movementSensitivity, scrollSensitivity, operationalModes);
        }
        return *instance;
    }

    /// Returns the singleton instance iff it has already been created.
    /// Otherwise, it throws an error saying that the mouse singleton
    /// has not been created.
    static auto getInstance() -> Mouse& {
        if (!instance) {
            throw std::runtime_error("Couldn't not get the mouse instance because instance has not been yet created.");
        }
        return *instance;
    }

    auto onNextFrame() const -> void {
        // std::printf("Mouse { .cursorPosition = (%f, %f), .lastCursorPosition = (%f, %f) }\n",
        //    cursorPosition.x, cursorPosition.y,
        //    lastCursorPosition.x, lastCursorPosition.y);
    }

    /// Must be called every frame after the camera and other objects
    /// that make use of the mouse user inputs.
    /// Sets the `lastCursorPosition` to the current `cursorPosition`.
    auto resetLastCursorPosition() -> void {
        lastCursorPosition = cursorPosition;
    }

    auto resetScrollOffset() -> void {
        scrollOffset *= 0.f;
    }

    [[nodiscard]] auto getOperationalModes() const -> std::uint8_t {
        return operationalModes;
    }

    auto setOperationalModes(const std::uint8_t operationalModes) -> void {
        this->operationalModes = operationalModes;
    }

    [[nodiscard]] auto inMode(const mouse::mode::ModeEnum mode) const -> bool {
        return operationalModes & mode;
    }

    auto enableMode(const mouse::mode::ModeEnum mode) -> void {
        operationalModes |= mode;
    }

    auto disableMode(const mouse::mode::ModeEnum mode) -> void {
        operationalModes &= ~(mode | std::uint8_t{0b00000000});
    }

    [[nodiscard]] auto getCursorPosition() const -> const glm::f64vec2& { return cursorPosition; }
    [[nodiscard]] auto getCursorPositionX() const -> glm::f64 { return cursorPosition.x; }
    [[nodiscard]] auto getCursorPositionY() const -> glm::f64 { return cursorPosition.y; }

    [[nodiscard]] auto getLastCursorPosition() const -> const glm::f64vec2& { return lastCursorPosition; }
    [[nodiscard]] auto getLastCursorPositionX() const -> glm::f64 { return lastCursorPosition.x; }
    [[nodiscard]] auto getLastCursorPositionY() const -> glm::f64 { return lastCursorPosition.y; }

    [[nodiscard]] auto getScrollOffset() const -> const glm::f64vec2& { return this->scrollOffset; }
    [[nodiscard]] auto getScrollOffsetX() const -> glm::f64 { return scrollOffset.x; }
    [[nodiscard]] auto getScrollOffsetY() const -> glm::f64 { return scrollOffset.y; }

    [[nodiscard]] auto getMovementSensitivity() const -> double { return movementSensitivity; }

    auto setMovementSensitivity(const double movementSensitivity) -> void {
        this->movementSensitivity = movementSensitivity;
    }

    [[nodiscard]] auto getScrollSensitivity() const -> double { return scrollSensitivity; }

    auto setScrollSensitivity(const double scrollSensitivity) -> void {
        this->scrollSensitivity = scrollSensitivity;
    }
};

Mouse* Mouse::instance = nullptr;
