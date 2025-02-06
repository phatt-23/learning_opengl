cmake_minimum_required(VERSION 3.30)

# Project name and versioning.
set(PROJECT_NAME 170cmake)
project(${PROJECT_NAME} VERSION 0.1)

# Set cmake and compiler settings.
set(CMAKE_CXX_STANDARD 23)

# Global include directory.
include_directories(./src/vendor)

# Creating executable.
add_executable(${PROJECT_NAME})

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE MODULE_SOURCES "src/*.cc")
file(GLOB_RECURSE VENDOR_SOURCES "src/vendor/*.cpp")

target_sources(${PROJECT_NAME} 
    PUBLIC 
        ${SOURCES} 
        ${VENDOR_SOURCES}
)

# Finding packages/libraries and linking them to the executable.
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(GLEW REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm::glm GLEW ${OPENGL_gl_LIBRARY})

# Create a library for all the modules and link it to the executable.
target_sources(${PROJECT_NAME} 
    PUBLIC 
        FILE_SET all_my_modules TYPE CXX_MODULES FILES 
        ${MODULE_SOURCES}
)

