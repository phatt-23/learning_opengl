//
// Created by phatt on 1/26/25.
//

module;

#include "std.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

export module timer;

/// Singleton class that provides the delta time of the running application.
export class Timer {
private:
    double deltaTime = 0.0f;
    double lastTime = 0.0f;

    Timer() = default;
    static Timer* singletonInstance;
public:
    /// Returns the singleton instance of this class.
    static auto getInstance() -> Timer& {
        if (singletonInstance == nullptr) {
            singletonInstance = new Timer();
        }
        return *singletonInstance;
    }

    /// Should be called on every frame so the delta time is recalculated.
    auto onNextFrame() -> void {
        const double currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
    }

    /// Returns the delta time of the frame where onNextFrame() was called the last time.
    [[nodiscard]] auto getDeltaTime() const -> double {
        return deltaTime;
    }

    /// Returns the delta time of the frame where onNextFrame() was called the last time.
    /// Cast to float.
    [[nodiscard]] auto f32getDeltaTime() const -> float {
        return static_cast<float>(deltaTime);
    }

    /// This should not be needed.
    static auto createInstance() -> Timer& {
        if (singletonInstance == nullptr) {
            singletonInstance = new Timer();
        }
        return *singletonInstance;
    }

    /// For the proper-proper singleton instance deletion.
    static auto deleteInstance() -> bool {
        if (singletonInstance == nullptr) {
            return false;
        }
        delete singletonInstance;
        return true;
    }
};

// Initialization of the singleton instance to null pointer.
Timer* Timer::singletonInstance = nullptr;
