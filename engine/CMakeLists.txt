cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Engine)

file(GLOB_RECURSE PROJECT_SOURCE_FILES CONFIGURE_DEPENDS "source/*.cpp"
     "source/*.h")

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
