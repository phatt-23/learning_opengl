//
// Created by phatt on 1/23/25.
//
module;

#include <glm/glm.hpp>
#include <GL/glew.h>
#include "std.h"
// #include "stb_image.h"

export module shader_program;

struct ShaderProgramSource {
    std::string vertexSource;
    std::string fragmentSource;
};

export class ShaderProgram {
    std::string filePath;
    GLuint shaderProgramID;
    std::unordered_map<std::string, GLint> uniformLocationsCache;
    std::unordered_map<std::string, GLint> attributeLocationsCache;

    /// Parses the shader source code containing both vertex and fragment shader sources.
    static auto parseShaderSource(const std::string& filePath) -> ShaderProgramSource {
        std::cout << "Parsing shader source: " << filePath << "\n";

        std::ifstream stream(filePath);
        if (!stream.is_open()) {
            throw std::runtime_error("Could not open file " + filePath);
        }

        enum ShaderType { VERTEX = 0, FRAGMENT = 1, NONE = -1 };
        ShaderType type = NONE;
        std::stringstream ss[2];
        std::string line;

        while (std::getline(stream, line)) {
            if (line.find("#shader") != std::string::npos) {
                if (line.find("vertex") != std::string::npos) {
                    type = VERTEX;
                } else if (line.find("fragment") != std::string::npos) {
                    type = FRAGMENT;
                }
            } else if (type != NONE) {
                ss[static_cast<int>(type)] << line << "\n";
            }
        }

        // std::println("Vertex Shader:\n{}", ss[static_cast<int>(VERTEX)].str());
        // std::println("Fragment Shader:\n{}", ss[static_cast<int>(FRAGMENT)].str());

        return ShaderProgramSource {
            .vertexSource = ss[static_cast<int>(VERTEX)].str(),
            .fragmentSource = ss[static_cast<int>(FRAGMENT)].str(),
        };
    }

    /// Compiles given shader source code - depends on shader type which can be either `vertex` of `fragment`.
    auto compileShader(const GLuint shaderType, const std::string& source) const -> GLuint {
        std::string typeString;
        switch (shaderType) {
            case GL_VERTEX_SHADER: { typeString = "vertex"; } break;
            case GL_FRAGMENT_SHADER: { typeString = "fragment"; } break;
            default: break;
        }
        std::cout << "Compiling shader source (" << typeString << "): " << filePath << "\n";
        // Create new vertex of fragment shader.
        const GLuint shaderID = glCreateShader(shaderType);
        // Feed in the source code to the shaderID (associate them).
        const char* sourcePointer = source.c_str();
        glShaderSource(shaderID, 1, &sourcePointer, nullptr);
        // Compile the shader.
        glCompileShader(shaderID);
        // Get the compilation result.
        GLint isCompiled;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
        // Check for errors.
        if (isCompiled == GL_FALSE) {
            int length;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
            const auto message = static_cast<char *>(alloca(length * sizeof(char)));
            glGetShaderInfoLog(shaderID, length, &length, message);
            glDeleteShader(shaderID);

            // Construct an error message.
            std::stringstream ss;
            ss << filePath << ": " << "Failed to compile ";
            ss << typeString << " shader:\n" << message;

            // and throw that shit.
            throw std::runtime_error(ss.str());
        }

        return shaderID;
    }

    /// Wraps the vertex shader and the fragment shaders into a shader program.
    auto createShaderProgramObject(const ShaderProgramSource& shaderSource) const -> GLuint {
        // Create a shader program.
        const GLuint shaderProgramID = glCreateProgram();
        // Compile the individual shaders.
        const GLuint vertexShaderID = compileShader(GL_VERTEX_SHADER, shaderSource.vertexSource);
        const GLuint fragmentShaderID = compileShader(GL_FRAGMENT_SHADER, shaderSource.fragmentSource);
        // Attach and link the individual shaders to the shader program.
        glAttachShader(shaderProgramID, vertexShaderID);
        glAttachShader(shaderProgramID, fragmentShaderID);
        glLinkProgram(shaderProgramID);
        glValidateProgram(shaderProgramID);
        // Now that they are linked, the shaders can be deleted.
        glDetachShader(shaderProgramID, vertexShaderID);
        glDeleteShader(vertexShaderID);
        glDetachShader(shaderProgramID, fragmentShaderID);
        glDeleteShader(fragmentShaderID);
        // Return the shader program ID.
        return shaderProgramID;
    }

public:
    explicit ShaderProgram(std::string shaderFilePath)
        : filePath(std::move(shaderFilePath)), shaderProgramID(0) {
        // Parse the source code containing both vertex and fragment shaders.
        const ShaderProgramSource sources = parseShaderSource(filePath);
        // Create a shader program out of them.
        this->shaderProgramID = createShaderProgramObject(sources);
    }

    /// Deconstructor that doesn't delete the 
    /// the shader program belonging to OpenGL.
    ~ShaderProgram() = default;

    /// Destroy the shader program.
    auto deleteProgram() -> void {
        glDeleteProgram(this->shaderProgramID);
        shaderProgramID = 0;
    }

    auto getSourceFilePath() const -> const std::string& {
        return filePath;
    }

    auto bind() const -> void {
        glUseProgram(this->shaderProgramID);
    }

    static auto unbind() -> void {
        glUseProgram(0);
    }

    /// Returns the location ID of a <b>uniform</b> variable in the shader program called `variableName`.
    /// If this uniform variable is not found, -1 is returned. \n
    /// CAUTION: Make sure to <b>bind</b> the shader program first.
    auto getUniformLocation(const std::string& variableName) -> GLint {
        // Check if the requested variable name was already requested.
        if (uniformLocationsCache.contains(variableName)) {
            // If yes, return the cached lookup result.
            return uniformLocationsCache[variableName];
        }
        // Else look it up in the shader program.
        const GLint location = glGetUniformLocation(shaderProgramID, variableName.c_str());
        if (location == -1) {
            // If it doesn't exist, report it.
            std::cerr << "Could not get location of uniform variable called '" << variableName
                      << "' in shader program of ID " << shaderProgramID << " ("<< filePath <<")" << "\n";
        }
        // Cache the result either way, even if the variable doesn't exist
        // so that in consecutive queries it won't do the lookup again.
        uniformLocationsCache[variableName] = location;
        // Return the variable's location.
        return location;
    }

    /// Returns the location ID of an <b>attribute</b> variable in the shader program called `variableName`.
    /// If this attribute variable is not found, -1 is returned. \n
    /// CAUTION: Make sure to <b>bind</b> the shader program first.
    auto getAttributeLocation(const std::string& variableName) -> GLint {
        // Check if the requested variable name was already requested.
        if (attributeLocationsCache.contains(variableName)) {
            // If yes, return the cached lookup result.
            return attributeLocationsCache[variableName];
        }
        // Else look it up in the shader program.
        const GLint location = glGetAttribLocation(shaderProgramID, variableName.c_str());
        if (location == -1) {
            // If it doesn't exist, report it.
            std::cerr << "Could not get location of attribute variable called '" << variableName
                      << "' in shader program of ID " << shaderProgramID << " ("<< filePath <<")" << "\n";
        }
        // Cache the result either way, even if the variable doesn't exist
        // so that in consecutive queries it won't do the lookup again.
        attributeLocationsCache[variableName] = location;
        // Return the variable's location.
        return location;
    }

    // Uniform variable setters
    /// Sets uniform int `variableName` in the shader program. \n
    /// CAUTION: Make sure to <b>bind</b> the shader program first.
    auto setUniform1i(const std::string& variableName, const GLint value) -> void {
        glUniform1i(getUniformLocation(variableName), value);
    }
    /// Sets uniform float `variableName` in the shader program. \n
    /// CAUTION: Make sure to <b>bind</b> the shader program first.
    auto setUniform1f(const std::string& variableName, const GLfloat value) -> void {
        glUniform1f(getUniformLocation(variableName), value);
    }
    /// Sets uniform vec3 `variableName` (float) in the shader program. \n
    /// CAUTION: Make sure to <b>bind</b> the shader program first.
    auto setUniform3f(const std::string& variableName, const glm::vec3& vector) -> void {
        glUniform3fv(getUniformLocation(variableName), 1, &vector[0]);
    }
    /// Sets uniform vec4 `variableName` (float) in the shader program. \n
    /// CAUTION: Make sure to <b>bind</b> the shader program first.
    auto setUniform4f(const std::string& variableName, const glm::vec4& vector) -> void {
        glUniform4fv(getUniformLocation(variableName), 1, &vector[0]);
    }
    /// Sets uniform mat4 `variableName` (float) in the shader program. \n
    /// CAUTION: Make sure to <b>bind</b> the shader program first.
    auto setUniformMat4f(const std::string& variableName, const glm::mat4& matrix) -> void {
        glUniformMatrix4fv(getUniformLocation(variableName), 1, GL_FALSE, &matrix[0][0]);
    }
};

