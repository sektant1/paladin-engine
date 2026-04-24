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
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm/glm/glm.hpp
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

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/json)

add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cgltf)

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
  miniaudio)

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${ASSETS_DIR} ${CMAKE_BINARY_DIR}/assets)
