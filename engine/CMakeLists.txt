cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Engine)

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h)

file(
  GLOB_RECURSE
  PROJECT_SOURCE_FILES
  CONFIGURE_DEPENDS
  "source/*.cpp"
  "source/*.h")

include_directories(source)
include_directories(thirdparty/glm)
option(ENGINE_UNITY_BUILD "Enable unity build for Engine" OFF)
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})
if(ENGINE_UNITY_BUILD)
  set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON UNITY_BUILD_BATCH_SIZE 16)
endif()
target_precompile_headers(
  ${PROJECT_NAME}
  PRIVATE
  <vector>
  <string>
  <memory>
  <unordered_map>
  <functional>
  <algorithm>
  <glm/glm.hpp>
  <GL/glew.h>)

add_subdirectory(thirdparty/glfw-3.4 "${CMAKE_CURRENT_BINARY_DIR}/glfw_build")
include_directories(thirdparty/glfw-3.4/include)
include_directories(thirdparty/stb_image)

add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE thirdparty/miniaudio)

if(UNIX AND NOT APPLE)
  target_link_libraries(miniaudio INTERFACE dl pthread m)
endif()

if(WIN32)
  target_link_libraries(miniaudio INTERFACE winmm user32 ole32)
endif()

if(APPLE)
  find_library(COREAUDIO_FRAMEWORK CoreAudio)
  find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
  find_library(AUDIOUNIT_FRAMEWORK AudioUnit)
  find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
  target_link_libraries(
    miniaudio
    INTERFACE ${COREAUDIO_FRAMEWORK}
              ${AUDIOUNIT_FRAMEWORK}
              ${AUDIOTOOLBOX_FRAMEWORK}
              ${COREFOUNDATION_FRAMEWORK})
endif()

# -- Third party: Dear ImGui (GLFW + OpenGL3 backend) --
add_library(
  imgui STATIC
  thirdparty/imgui/imgui.cpp
  thirdparty/imgui/imgui_demo.cpp
  thirdparty/imgui/imgui_draw.cpp
  thirdparty/imgui/imgui_tables.cpp
  thirdparty/imgui/imgui_widgets.cpp
  thirdparty/imgui/backends/imgui_impl_glfw.cpp
  thirdparty/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui
                                        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends)

target_link_libraries(imgui PRIVATE ${GLFW_LIB} glew)

# -- Third party: Catch2 (vendored) --
option(CHAD_BUILD_TESTS "Build chad-engine tests" ON)
if(CHAD_BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/catch2/CMakeLists.txt)
  set(CATCH_INSTALL_DOCS
      OFF
      CACHE BOOL "" FORCE)
  set(CATCH_INSTALL_EXTRAS
      OFF
      CACHE BOOL "" FORCE)
  add_subdirectory(thirdparty/catch2)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/catch2/extras)
  enable_testing()
endif()

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/json)

add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cgltf)

# -- Third party: Assimp (FBX + glTF + OBJ importers only) --
set(ASSIMP_BUILD_TESTS                    OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS             OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES                  OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL                        OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL_PDB                    OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT                      ON  CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS             OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB                     ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER             ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER            ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER             ON  CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/assimp EXCLUDE_FROM_ALL)

set(BUILD_UTILS
    OFF
    CACHE BOOL "utilities" FORCE)
add_subdirectory(thirdparty/glew/build/cmake "${CMAKE_CURRENT_BINARY_DIR}/glew_build")
include_directories(thirdparty/glew/include)
set_property(TARGET glew_s PROPERTY FOLDER GLEW)
set_property(TARGET glew PROPERTY FOLDER GLEW)
set_target_properties(glew PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})

set(BUILD_UNIT_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_BULLET3
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_EXTRAS
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_ENET
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET
    OFF
    CACHE BOOL "" FORCE)
set(USE_GRAPHICAL_BENCHMARK
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/bullet EXCLUDE_FROM_ALL)
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty/bullet/src)

target_link_libraries(
  ${PROJECT_NAME}
  glfw
  glew_s
  nlohmann_json
  cgltf
  glm
  BulletDynamics
  BulletCollision
  LinearMath
  miniaudio
  imgui
  assimp)

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${ASSETS_DIR} ${CMAKE_BINARY_DIR}/assets)
