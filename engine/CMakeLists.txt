cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Engine)

set(PROJECT_SOURCE_FILES
    source/utils/FileReader.h
    source/utils/FileReader.cpp
    source/Types.h
    source/Log.h
    source/Engine.h
    source/Engine.cpp
    source/Application.h
    source/Application.cpp
    source/input/InputManager.h
    source/input/InputManager.cpp
    source/graphics/ShaderProgram.h
    source/graphics/ShaderProgram.cpp
    source/graphics/GraphicsAPI.h
    source/graphics/GraphicsAPI.cpp
    source/render/Material.h
    source/render/Material.cpp
    source/render/Builder.h
    source/render/Builder.cpp
    source/render/Mesh.h
    source/render/Mesh.cpp
    source/render/RenderQueue.h
    source/render/RenderQueue.cpp)

include_directories(source)
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

# Add GLFW library
add_subdirectory(thirdparty/glfw-3.4 "${CMAKE_CURRENT_BINARY_DIR}/glfw_build")
include_directories(thirdparty/glfw-3.4/include)

# Add Glew library
set(BUILD_UTILS
    OFF
    CACHE BOOL "utilities" FORCE)
add_subdirectory(thirdparty/glew/build/cmake
                 "${CMAKE_CURRENT_BINARY_DIR}/glew_build")
include_directories(thirdparty/glew/include)
set_property(TARGET glew_s PROPERTY FOLDER GLEW)
set_property(TARGET glew PROPERTY FOLDER GLEW)

# Link all thirdparty libraries
target_link_libraries(${PROJECT_NAME} glfw glew_s)
