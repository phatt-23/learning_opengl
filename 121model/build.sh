#!/bin/bash

# NOTE: clang compiler doesn't support module partitions
# (e.g. `module maths:calc`, must be written as `module maths.calc` for now).

# Directory structure
SOURCE_DIR="src"
VENDOR_DIR="src/vendor"
BUILD_DIR="build"
OBJECT_DIR="build/obj"
PREBUILT_MODULES_DIR="build/pcm"
PREBUILT_CACHE_DIR="build/pcm/cache"
PROGRAM_EXECUTABLE_PATH="build/program"

# File extensions
CPP_FILE_EXTENSION=".cpp"
CPP_MODULE_FILE_EXTENSION=".cc"

# Compilation and Linking Flags
LANGUAGE_STANDARD="-std=c++23 -stdlib=libc++ -I/usr/include/c++/v1/"
INCLUDE_DIRECTORIES="-I./src/vendor"
COMMON_FLAGS="$LANGUAGE_STANDARD $INCLUDE_DIRECTORIES \
    -fmodules -fbuiltin-module-map \
    -fprebuilt-implicit-modules -fprebuilt-module-path=$PREBUILT_MODULES_DIR \
    -fmodules-cache-path=$PREBUILT_CACHE_DIR"
PCM_MODULE_FLAGS="-Xclang -emit-module-interface"
LINKING_FLAGS="-lglfw -lGL -lGLEW -lX11 -lGLU -lOpenGL -lpthread -ldl -lXrandr -lassimp"

# Helper function that takes in a .cc module file
# (extension can be specified by $CPP_MODULE_FILE_EXTENSION variable)
# and creates .pcm precompiled module interface and object file.
# It puts this .pcm file into $PREBUILT_MODULES_DIR
# and the object file into $OBJECT_DIR
# As its parameter it only takes in the name of the module itself,
# not the name of the file it written in.
function compile_module_into_pcm_and_object_file {
    echo "compile_module_into_pcm_and_object_file $1"
    # Prebuilds the module file into a .pcm prebuilt module file.
    clang++ -o $PREBUILT_MODULES_DIR/$1.pcm -c $SOURCE_DIR/$1$CPP_MODULE_FILE_EXTENSION $COMMON_FLAGS $PCM_MODULE_FLAGS
    # Compiles the module file into an .o object file.
	clang++ -o $OBJECT_DIR/$1$CPP_MODULE_FILE_EXTENSION.o -c $SOURCE_DIR/$1$CPP_MODULE_FILE_EXTENSION $COMMON_FLAGS
}

# Normal C++ source file compilation into an object file.
# They end with an extension specified by $CPP_FILE_EXTENSION
function compile_cpp_into_object_file {
    echo "compile_cpp_into_object_file $1"
    clang++ -o $OBJECT_DIR/$1$CPP_FILE_EXTENSION.o -c $SOURCE_DIR/$1$CPP_FILE_EXTENSION $COMMON_FLAGS
}

# Linking the object files and libraries into executable.
function link_to_executable_routine {
    clang++ -o $PROGRAM_EXECUTABLE_PATH $(find $OBJECT_DIR/*.o) $COMMON_FLAGS $LINKING_FLAGS
}

# Precompiles files that are unlikely to change (standard libraries, vendor libraries).
# If needed you can run this routine by yourself by running "precomp".
function precompile_routine {
    # Compile the standard library.
    compile_cpp_into_object_file std
    # Compiling vendor sources.
    clang++ -c $VENDOR_DIR/stb_image.cpp -o $OBJECT_DIR/stb_image.cpp.o $COMMON_FLAGS
}

# Runs build routine - compilation and linking.
# When compiling modules, make sure to keep the correct topological ordering.
# I'm listing out module dependencies before the function calls.
function build_routine {
    # none 
    compile_module_into_pcm_and_object_file timer
    compile_module_into_pcm_and_object_file mouse
    compile_module_into_pcm_and_object_file shader_program
    compile_module_into_pcm_and_object_file index_buffer
    compile_module_into_pcm_and_object_file vertex_buffer.supported_types
    compile_module_into_pcm_and_object_file vertex_buffer.layout
    # vertex_buffer.layout
    compile_module_into_pcm_and_object_file vertex_buffer.vertex_struct
    # vertex_buffer.supported_types vertex_buffer.layout vertex_buffer.vertex_struct
    compile_module_into_pcm_and_object_file vertex_buffer
    # shader_program
    compile_module_into_pcm_and_object_file transformation
    # shader_program
    compile_module_into_pcm_and_object_file texture
    # shader_program mouse
    compile_module_into_pcm_and_object_file camera
    # vertex_buffer index_buffer
    compile_module_into_pcm_and_object_file vertex_array
    # vertex_array 
    compile_module_into_pcm_and_object_file mesh
    # mesh
    compile_module_into_pcm_and_object_file model 
    # everything
    compile_module_into_pcm_and_object_file application
    
    compile_cpp_into_object_file main 

    link_to_executable_routine
}

# Setup directories
function setup_routine {
	mkdir -p $BUILD_DIR $OBJECT_DIR $PREBUILT_MODULES_DIR $PREBUILT_CACHE_DIR
}

# Clean build artifacts
function clean_routine {
	rm -rf $BUILD_DIR
}

# Run the program
function run_routine {
    ./$PROGRAM_EXECUTABLE_PATH
}

echo "You called the rule '$1'"

case "$1" in
    "build" | "b")
        echo "Building the project."
        setup_routine
        build_routine
        ;;
    "")
        echo "Implicitly building the project."
        setup_routine
        build_routine
        ;;
    "setup" | "s")
        echo "Setup the directories."
        setup_routine
        ;;
    "clean" | "c")
        echo "Cleaning the build directory."
        clean_routine
        ;;
    "run" | "r")
        echo "Running the executable."
        run_routine
        ;;
    "link" | "l")
        echo "Linking object files to executable."
        link_to_executable_routine
        ;;
    "mod" | "m")
        echo "Compiling module '$2' (if it exists)."
        compile_module_into_pcm_and_object_file $2
        ;;
    "modlink" | "ml")
        echo "Compiling module '$2' (if it exists)."
        compile_module_into_pcm_and_object_file $2
        echo "Linking object files to executable."
        link_to_executable_routine
        ;;
    "modlinkrun" | "mlr")
        echo "Compiling module '$2' (if it exists)."
        compile_module_into_pcm_and_object_file $2
        echo "Linking object files to executable."
        link_to_executable_routine
        echo "Running the executable."
        run_routine
        ;;
    "buildrun" | "br")
        echo "Building the project."
        setup_routine
        build_routine
        echo "Running the executable."
        run_routine
        ;;
    "linkrun" | "lr")
        echo "Linking object files to executable."
        link_to_executable_routine
        echo "Running the executable."
        run_routine
        ;;
    "precomp")
        echo "Precompiling."
        precompile_routine
        ;;
    *)
        echo "Unknown routine."
        ;;
esac

