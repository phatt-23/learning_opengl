cmake_minimum_required(VERSION 3.30)

# Project name and versioning.
set(PROJECT_NAME 140stencil)
project(${PROJECT_NAME} VERSION 0.1)

# Set cmake and compiler settings.
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(OpenGL_GL_PREFERENCE LEGACY)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# Global include directory.
include_directories(./src/vendor)

# Creating executable.
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# Finding packages/libraries and linking them to the executable.
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(GLEW REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm::glm GLEW ${OPENGL_gl_LIBRARY})

# Create a library for all the modules and link it to the executable.
add_library(my_cxx_modules)
file(GLOB_RECURSE MODULE_SOURCES "src/*.cc")
target_sources(my_cxx_modules PUBLIC FILE_SET CXX_MODULES FILES ${MODULE_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC my_cxx_modules)

