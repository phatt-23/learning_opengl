//
// Created by phatt on 1/23/25.
//

module;

#include "std.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

// Vertices coordinates
auto floorVertices = std::vector<Vertex> {
    //      COORDINATE              NORMAL              COLOR               TEXCOORD
    Vertex( {-1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} ),
	Vertex( {-1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} ),
	Vertex( {1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} ),
	Vertex( {1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} ),
};

// Indices for vertices order
auto floorIndices = std::vector<GLuint> {
    0, 1, 2,
	0, 2, 3,
};

auto lightVertices = std::vector<Vertex> {
    // COORDINATES
    Vertex( {-0.1f, -0.1f,  0.1f} ),
	Vertex( {-0.1f, -0.1f, -0.1f} ),
	Vertex( { 0.1f, -0.1f, -0.1f} ),
	Vertex( { 0.1f, -0.1f,  0.1f} ),
	Vertex( {-0.1f,  0.1f,  0.1f} ),
	Vertex( {-0.1f,  0.1f, -0.1f} ),
	Vertex( { 0.1f,  0.1f, -0.1f} ),
	Vertex( { 0.1f,  0.1f,  0.1f} ),
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
        glDepthFunc(GL_LEQUAL);
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

        // Creating texture object for the floor mesh.
        auto floorTextures = std::vector<Texture> {
            Texture("./textures/planks.png", texture::Dimension::$2D, texture::Type::DiffuseMap, texture::DataFormat::RGBA, 0),
            Texture("./textures/planksSpec.png", texture::Dimension::$2D, texture::Type::SpecularMap, texture::DataFormat::R, 1),
        };

    	// The floor mesh (has its own VAO) and the shader program it will use.
        auto floorMesh = Mesh(floorVertices, floorIndices, floorTextures);
        auto floorShader = ShaderProgram("./shaders/floor.glsl");

		// The light cube mesh and the shader program it will use.
        auto lightCubeMesh = Mesh(lightVertices, lightIndices, {});
    	auto lightCubeShader = ShaderProgram("./shaders/light_cube.glsl");

    	auto lightCubeColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    	auto lightCubeModelMat = glm::mat4(1.0f);
    	auto lightCubePositionVec = glm::vec3(0.5f, 0.5f, 0.5f);
    	auto lightCubeTranslationMat = glm::translate(lightCubeModelMat, lightCubePositionVec);
    	lightCubeModelMat = lightCubeTranslationMat;

		lightCubeShader.bind();
    	lightCubeShader.setUniform4f("U_ColorVec4", lightCubeColor);
		lightCubeShader.setUniformMat4f("U_ModelMat4", lightCubeModelMat);
		ShaderProgram::unbind();

    	floorShader.bind();
    	floorShader.setUniform4f("U_LightColorVec4", lightCubeColor);
    	floorShader.setUniform3f("U_LightPositionVec3", lightCubePositionVec);
    	ShaderProgram::unbind();

        // Rotation of the object using the model matrix
        float rotationInDegrees = 0.0f;

        while (!glfwWindowShouldClose(this->window)) {
            this->onNextFrame(); // Poll events, handle resizing of the window
            Timer::getInstance().onNextFrame(); // Update the delta time for the current frame.
            Mouse::getInstance().onNextFrame(); // Does not reset the last cursor.
            // Handles user input and update the camera's position and orientation (and proj-veiw matrix).
            camera.onNextFrame(this->window, Timer::getInstance().getDeltaTime()); 
            // Resets the mouse after all of its user are done using it. TODO: Observer pattern.
            Mouse::getInstance().resetLastCursorPosition();

            // Update objects in the scene
            this->onUpdate();

        	// Send the floor model matrix to the shader code.
            auto rotationMat = glm::mat4(1.0f);
            auto translationMat = glm::mat4(1.0f);
            rotationInDegrees += 10.f * Timer::getInstance().f32getDeltaTime();
            rotationMat = glm::rotate(rotationMat, glm::radians(rotationInDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
            translationMat = glm::translate(translationMat, glm::vec3(0.0f, 0.3f, 0.0f));
            auto modelMat = translationMat * rotationMat;
            floorShader.bind();
            floorShader.setUniformMat4f("U_ModelMat4", modelMat);
            ShaderProgram::unbind();

        	// Draw the floor.
            floorMesh.draw(floorShader, camera);

			// Draw the light cube.
            lightCubeMesh.draw(lightCubeShader, camera);

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

