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
    static auto getInstance() -> Timer& {
        if (singletonInstance == nullptr) {
            singletonInstance = new Timer();
        }
        return *singletonInstance;
    }

    auto onNextFrame() -> void {
        const double currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
    }

    [[nodiscard]] auto getDeltaTime() const -> double {
        return deltaTime;
    }
};

Timer* Timer::singletonInstance = nullptr;
