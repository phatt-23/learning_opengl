//
// Created by phatt on 1/23/25.
//

module;

#include "std.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

export module application;

import shader_program;
import vertex_array;
import index_buffer;
import vertex_buffer;
import texture;
import camera;
import timer;
import mouse;
import mesh;
import model;
import transformation;

auto lightVertices = std::vector<Vertex> {
    Vertex{ {-0.1f, -0.1f,  0.1f} },
	Vertex{ {-0.1f, -0.1f, -0.1f} },
	Vertex{ { 0.1f, -0.1f, -0.1f} },
	Vertex{ { 0.1f, -0.1f,  0.1f} },
	Vertex{ {-0.1f,  0.1f,  0.1f} },
	Vertex{ {-0.1f,  0.1f, -0.1f} },
	Vertex{ { 0.1f,  0.1f, -0.1f} },
	Vertex{ { 0.1f,  0.1f,  0.1f} },
};

auto lightIndices = std::vector<GLuint> {
    0, 1, 2,
	0, 2, 3,
	0, 4, 7,
	0, 7, 3,
	3, 7, 6,
	3, 6, 2,
	2, 6, 5,
	2, 5, 1,
	1, 5, 4,
	1, 4, 0,
	4, 5, 6,
	4, 6, 7,
};

auto transparentPositions = std::vector<glm::vec3> {
    glm::vec3(-1.5f,  0.0f, -0.48f),
    glm::vec3( 1.5f,  0.0f,  0.51f),
    glm::vec3( 0.0f,  0.0f,  0.7f),
    glm::vec3(-0.3f,  0.0f, -2.3f),
    glm::vec3( 0.5f,  0.0f, -0.6f),
};

auto transparentVertices = std::vector<Vertex> {
    // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
    Vertex{ .position = {0.0f,  0.5f,  0.0f},  .texUV = {0.0f,  0.0f} },
    Vertex{ .position = {0.0f, -0.5f,  0.0f},  .texUV = {0.0f,  1.0f} },
    Vertex{ .position = {1.0f, -0.5f,  0.0f},  .texUV = {1.0f,  1.0f} },
    Vertex{ .position = {1.0f,  0.5f,  0.0f},  .texUV = {1.0f,  0.0f} }
};

auto transparentIndices = std::vector<GLuint> {
    0, 1, 2,
    0, 2, 3,
};

auto floorVertices = std::vector<Vertex> {
    Vertex{ {-1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
	Vertex{ {-1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
	Vertex{ {1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
	Vertex{ {1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
};

auto floorIndices = std::vector<GLuint> {
    0, 1, 2,
	0, 2, 3,
};


export class Application
{
private:
    std::string windowTitle;
    GLFWwindow* window;
    glm::i32vec2 displayDimensions;
public:
    Application(std::string windowTitle, const int windowWidth, const int windowHeight)
    : windowTitle(std::move(windowTitle)), 
    window(nullptr), 
    displayDimensions(windowWidth, windowHeight) {}

    ~Application() = default;

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
        glDepthFunc(GL_LESS);

        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK); // Cull back faces (default)
        // glFrontFace(GL_CW); // Ensure correct winding order

        // glDepthMask(GL_FALSE);
        glEnable(GL_STENCIL_TEST);
    }

    // Sets up thing that are needed - camera, ...
    auto setUp() -> void {
    }

    // Represents the main game loop.
    auto mainLoop() -> void {
        // Create camera object.
        auto camera = Camera(this->window, 2.f, glm::vec3(0.0f, 0.0f, 4.0f));
        // Create mouse singleton instance.
        Mouse::createInstance(this->window, 0.6f);

        auto modelShader = ShaderProgram("./shaders/model_with_light.glsl");
        auto lightShader = ShaderProgram("./shaders/light_cube.glsl");
        auto floorShader = ShaderProgram("./shaders/floor.glsl");
        auto singleColorShader = ShaderProgram("./shaders/single_color.glsl");
        auto blendingShader = ShaderProgram("./shaders/blending.glsl");


        // Create a model from file.
        // auto model = Model("./models/grindstone/scene.gltf");
        // auto model = Model("./models/lingerie_girl/scene.gltf");
        // auto model = Model("./models/the_girl_on_the_floor/scene.gltf");
        // auto model = Model("./models/the_girl_on_the_floor_v2/scene.gltf");
        auto model = Model("./models/goddess_white_voluptuous/scene.gltf");
        // auto model = Model("./models/girl_in_lingerie/scene.gltf");
        // auto model = Model("./models/alleyana/scene.gltf");
        // auto model = Model("./models/scimitar/scene.gltf");
        // auto model = Model("./models/stylized_ww1_plane/scene.gltf");
        // auto model = Model("./models/scifi_girl_v.01/scene.gltf");
        // auto model = Model("./models/girl_model_2/scene.gltf");

        auto lightMesh = Mesh(lightVertices, lightIndices, {});
        const auto lightPosition = glm::f32vec3(0.0, 0.4, -1.0);
        const auto lightColor = glm::f32vec4(1.0, 1.0, 1.0, 1.0);


        auto floorMesh = Mesh(floorVertices, floorIndices, {
            Texture("./textures/planks.png", texture::Dimension::$2D, texture::Type::DiffuseMap, texture::DataFormat::RGBA),
            Texture("./textures/planksSpec.png", texture::Dimension::$2D, texture::Type::SpecularMap, texture::DataFormat::R),
        });



        auto transparentWindowMesh = Mesh(transparentVertices, transparentIndices, {
            Texture("./textures/blending_transparent_window.png", texture::Dimension::$2D, texture::Type::DiffuseMap, texture::DataFormat::RGBA),
        });



        lightShader.bind();
        lightShader.setUniform4f("U_LightColorVec4", lightColor);

        modelShader.bind();
        modelShader.setUniform4f("U_LightColorVec4", lightColor);
        modelShader.setUniform3f("U_LightPositionVec3", lightPosition);
        
    	floorShader.bind();
    	floorShader.setUniform4f("U_LightColorVec4", lightColor);
    	floorShader.setUniform3f("U_LightPositionVec3", lightPosition);





        float rotationInDegrees = 0.f;

        while (!glfwWindowShouldClose(this->window)) {
            this->onNextFrame(); // Poll events, handle resizing of the window
            Timer::getInstance().onNextFrame(); // Update the delta time for the current frame.
            Mouse::getInstance().onNextFrame(); // Does not reset the last cursor.
            // Handles user input and update the camera's position and orientation (and proj-view matrix).
            camera.onNextFrame(this->window, Timer::getInstance().getDeltaTime()); 
            // Resets the mouse after all of its user are done using it. TODO: Observer pattern.
            Mouse::getInstance().resetLastCursorPosition();
            
            rotationInDegrees += Timer::getInstance().f32getDeltaTime() * 30.f;

            // Update objects in the scene
            this->onUpdate();

            const auto modelTransform = Transformation(
                    {0.0, 0.4, 0.0},
                    {0, 1, 0}, rotationInDegrees,
                    {0.6, 0.6, 0.6} );
            [[maybe_unused]]
            const auto scaledUpModelTransform = Transformation(
                    {0.0, 0.4, 0.0},
                    {0, 1, 0}, rotationInDegrees,
                    {0.7, 0.7, 0.7} );
            const auto lightTransform = Transformation(
                    lightPosition,
                    {0.0, 1.0, 0.0}, 0.0,
                    {0.2, 0.2, 0.2} );
            const auto floorTransform = Transformation(
                    {0.0, -0.2, 0.0},
                    {0.0, 1.0, 0.0}, 0.0,
                    {1.0, 1.0, 1.0} );


            // If the fragment passes then replace the stencil value.
            // Otherwise keep the original stencil value.
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

            // Disable all the masks. There is in total 256 stencil masks
            // available. By setting the mask to 0x00 the stencil will
            // about to be replaced is ANDed with zeros so no mask
            // is overwritten.
            glStencilMask(0x00);

            // Draw the objects without outline normally.
            lightMesh.draw(lightShader, camera, lightTransform);
            floorMesh.draw(floorShader, camera, floorTransform);

            // Sort transparent object from farthest to closest to the eye.
            std::sort(transparentPositions.begin(), transparentPositions.end(),
                      [&camera](const glm::vec3& x, const glm::vec3& y) {
                          return glm::length(camera.getPosition() - x) >= glm::length(camera.getPosition() - y);
                      });

            // Draw transparent object after all the opaque ones were drawn.
            for (const glm::vec3& position : transparentPositions) {
                const auto transparentWindowTransformation = Transformation(
                        position,
                        {0.0, 1.0, 0.0}, 0.0,
                        glm::vec3(1.0));
                transparentWindowMesh.draw(blendingShader, camera, transparentWindowTransformation);
            }

            // Make the stencil test pass everytime.
            // The stencil buffer is initially filled with zeros.
            // And now we always allow writing to the stencil buffer.
            // And when it passes we replace the underlying value (0)
            // with the reference value (1). The mask of 0xFF makes
            // it not restrict any bits.
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            // Enable all the stencil masks.
            glStencilMask(0xFF);

            // Draw the model normally. Now where the fragments are shaded by
            // the model, the stencil value is set to 1.
            model.draw(modelShader, camera, modelTransform);

            // Disable writing to the stencil mask.
            glStencilMask(0x00);
            // Make the stencil function pass only when the underlying stencil value
            // is not equal to the reference value. Make the mask non-restrictive.
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            // Disable depth testing to the outline will always be on top.
            glDisable(GL_DEPTH_TEST);

            // Draw the object scaled up with one color. Making a 'border' around the object.
            model.draw(singleColorShader, camera, scaledUpModelTransform);

            // Allow writing to the stencil mask.
            glStencilMask(0xFF);
            // Re-enable depth testing.
            glEnable(GL_DEPTH_TEST);




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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    // Updates the objects in the game loop - position of objects, camera.
    // Called on every iteration of the main game loop.
    // Also draw them out to the framebuffer.
    auto onUpdate() -> void {
    }

    /// Renders the objects to the window.
    /// Called on every iteration of the main game loop.
    /// Swaps the frame buffers.
    auto onRender() -> void {
        glfwSwapBuffers(this->window);
    }

};

