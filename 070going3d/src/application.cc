//
// Created by phatt on 1/23/25.
//

module;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "std.h"

export module application;

import shader_program;
import vertex_array;
import index_buffer;
import vertex_buffer;
import texture;
import camera;

export class Application
{
private:
    std::string windowTitle;
    GLFWwindow* window;
    glm::i32vec2 displayDimensions;
public:
    Application(std::string windowTitle, const int windowWidth, const int windowHeight)
        : windowTitle(std::move(windowTitle)), window(nullptr), displayDimensions(windowWidth, windowHeight)
    {}

    ~Application() {}

    // Runs the Application.
    auto run() -> void {
        this->initialize();
        this->setUp();
        this->mainLoop();
        this->cleanUp();
    }
private:
    // Initializes needed libraries.
    auto initialize() -> void {
        // GLFW initialisation
        // Initialize GLFW that will create a window
        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error("Failed to initialize GLFW3");
        }

        // Tell GLFW what OpenGL version we are using.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        // Tell GLFW we will only use the core functionality
        // and not compatibility mode which includes outdated functions.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        // Create GLFW window.
        this->window = glfwCreateWindow(
            this->displayDimensions.x,
            this->displayDimensions.y,
            this->windowTitle.data(),
            nullptr,
            nullptr);

        if (this->window == nullptr) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        // Introduce the created window to GLFW context.
        glfwMakeContextCurrent(this->window);

        // Set how long will the pause be between each frame swaps.
        glfwSwapInterval(1);

        // Set a frame-buffer size callback for resizable GLFW window.
        // This callback is called everytime the window is resized.
        glfwSetFramebufferSizeCallback(this->window,
            [](GLFWwindow* window, const int width, const int height) -> void {
                std::cout << "Window resized (x: " << width << ", y: " << height << ")\n";
                // in case that there are multiple windows opened, make this one the current
                glfwMakeContextCurrent(window);
                // make GLFW to render the GLFWwindow from x=0 to x=width and y=0 to y=height
                glViewport(0, 0, width, height);
            });

        // OpenGL
        // Initialize GLEW which will prepare OpenGL functions prototypes that are implemented on the GPU.
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("Failed to initialize GLEW");
        }

        // Enable alpha blending (also how will the blending be done).
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Enable depth testing / z-buffer is big as the color buffer.
        glEnable(GL_DEPTH_TEST);
        // If the fragment's depth value is less than the stored depth value the fragment's color passes.
        // By default, this is set to GL_LESS.
        glDepthFunc(GL_LEQUAL);
    }

    // Sets up thing that are needed - camera, ...
    auto setUp() -> void {

    }

    // Represents the main game loop.
    auto mainLoop() -> void {
        // Describes how OpenGL should interpret the array of values below.
        const auto layout = VertexBufferLayout()
            .pushAttribute<GLfloat>(3, "AV_PositionVec3")
            .pushAttribute<GLfloat>(3, "AV_ColorVec3")
            .pushAttribute<GLfloat>(2, "AV_TextureCoordinateVec2");
        // Vertex coordinates that will be put into VBO.
        const auto vertexAttributes = std::initializer_list {
            -0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	0.0f, 0.0f,
            -0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	5.0f, 0.0f,
            0.5f, 0.0f, -0.5f,      0.83f, 0.70f, 0.44f,    0.0f, 0.0f,
            0.5f, 0.0f,  0.5f,      0.83f, 0.70f, 0.44f,    5.0f, 0.0f,
            0.0f, 0.8f,  0.0f,      0.92f, 0.86f, 0.76f,    2.5f, 5.0f
        };

        // Indices that will be put into IBO.
        const auto indices = std::initializer_list<GLuint> {
            0, 1, 2,
            0, 2, 3,
            0, 1, 4,
            1, 2, 4,
            2, 3, 4,
            3, 0, 4
        };

        // Create the shader program from the glsl source code.
        auto shaderProgram = ShaderProgram("./shaders/basic_3d_shader.glsl");
        const auto vertexBuffer = VertexBuffer(data(vertexAttributes), vertexAttributes.size());
        const auto indexBuffer = IndexBuffer(data(indices), indices.size());
        const auto vertexArray = VertexArray();
        vertexArray.linkVertexBufferWithIndices(vertexBuffer, layout, indexBuffer);

        const auto texture = Texture("./textures/ISO_C++_Logo.png", GL_TEXTURE_2D, 0);

        // Make sure to bind the shader program first.
        shaderProgram.bind();
        // Since we are using GL_TEXTURE0 we assign '0' to the sampler2D
        // variable representing the texture unit slot at 0-th index.
        shaderProgram.setUniform1i("U_Texture0", 0);
        ShaderProgram::unbind();

        // Rotation of the object using the model matrix
        float rotationInDegrees = 0.0f;        
        double previousTime = glfwGetTime();

        while (!glfwWindowShouldClose(this->window)) {
            // Poll events, handle resizing of the window
            this->onNextFrame();
            // Update objects in the scene
            this->onUpdate();

            // Every 1/60 seconds add half a degree to the rotation.
            if (const double currentTime = glfwGetTime(); currentTime - previousTime >= 1/60) {
                rotationInDegrees += 0.5f;
                previousTime = currentTime;
            }

            // The object is in tha same position as it was
            // because these are unit vectors.
            auto model = glm::mat4(1.0f);
            auto view = glm::mat4(1.0f);
            auto proj = glm::mat4(1.0f);
            // Make the model rotate by some amount around specified axis.
            model = glm::rotate(model, glm::radians(rotationInDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
            // Moving the object lower (y coord) and closer (z coord).
            view = glm::translate(view, glm::vec3(0.0f, -0.5f, -2.0f));
            // Setting FOV, specifying aspect ratio, near and far plane.
            proj = glm::perspective(glm::radians(45.0f), float(displayDimensions.x / displayDimensions.y), 0.1f, 100.0f);
            // Send these matrices to the GPU vertex shader code and let it calculate where each vertex will go to.
            shaderProgram.bind();
            shaderProgram.setUniformMat4f("U_ModelMat4", model);
            shaderProgram.setUniformMat4f("U_ViewMat4", view);
            shaderProgram.setUniformMat4f("U_ProjectionMat4", proj);
            ShaderProgram::unbind();

            // Bind the shader we wish to use to draw the VAO to the screen.
            shaderProgram.bind();
            // Bind texture object that we want to use.
            texture.bind();
            // Bind VAO to draw the VBO it holds according to the IBO's ordering.
            vertexArray.bind();
            // Bind the VAO. You don't have to bing anything else.
            glDrawElements(GL_TRIANGLES, indexBuffer.getElementCount(), GL_UNSIGNED_INT, nullptr);
            // Unbind the shader program, texture object and vertex array after drawing.
            ShaderProgram::unbind();
            Texture::unbind();
            VertexArray::unbind();

            // Render the objects to the window
            this->onRender();
        }
    }

    // Destroys objects and frees the memory.
    auto cleanUp() const -> void {
        glfwDestroyWindow(this->window); // Destroy and free the GLFW window
        glfwTerminate(); // Shutdown GLFW altogether
    }

    // Does the basic stuff - poll events, handling window resize, clearing the screen.
    // Called on every iteration of the main game loop.
    auto onNextFrame() -> void {
        glfwPollEvents(); // Poll for user events - key presses, mouse movements, window close, ...
        // Update the displayDimensions member i32vec2
        glfwGetFramebufferSize(this->window, &this->displayDimensions.x, &this->displayDimensions.y);
        // Clear the window by a provided color.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // Clear the color buffer bit and the depth buffer bit (aka z-buffer)
        // for the new frame to have a clean canvas to draw and depth test on.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Updates the objects in the game loop - position of objects, camera.
    // Called on every iteration of the main game loop.
    auto onUpdate() -> void {

    }

    // Renders the objects to the window.
    // Called on every iteration of the main game loop.
    auto onRender() -> void {
        glfwSwapBuffers(this->window);
    }

};

