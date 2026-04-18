cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Engine)

set(PROJECT_SOURCE_FILES
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
	source/render/Mesh.h
	source/render/Mesh.cpp
	source/render/RenderQueue.h
	source/render/RenderQueue.cpp
	source/scene/GameObject.h
	source/scene/GameObject.cpp
	source/scene/Scene.h
	source/scene/Scene.cpp
	source/scene/Component.h
	source/scene/Component.cpp
	source/scene/components/MeshComponent.h
	source/scene/components/MeshComponent.cpp
	source/scene/components/CameraComponent.h
	source/scene/components/CameraComponent.cpp
)

# Group files according to files organization
foreach(FILE ${PROJECT_SOURCE_FILES}) 
    # Get the directory of the source file
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

    # Remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

    # Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")

    source_group("${GROUP}" FILES "${FILE}")
endforeach()

include_directories(source)
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

# Add GLFW library
add_subdirectory(thirdparty/glfw-3.4 "${CMAKE_CURRENT_BINARY_DIR}/glfw_build")
include_directories(thirdparty/glfw-3.4/include)

# Add Glew library
set(BUILD_UTILS OFF CACHE BOOL "utilities" FORCE)
add_subdirectory(thirdparty/glew/build/cmake "${CMAKE_CURRENT_BINARY_DIR}/glew_build")
include_directories(thirdparty/glew/include)
set_property(TARGET glew_s PROPERTY FOLDER GLEW)
set_property(TARGET glew PROPERTY FOLDER GLEW)

# Add GLM library
add_subdirectory(thirdparty/glm-1.0.1/glm)
include_directories(thirdparty/glm-1.0.1)

# Link all thirdparty libraries
target_link_libraries(${PROJECT_NAME} 
    glfw 
    glew_s
	glm
)