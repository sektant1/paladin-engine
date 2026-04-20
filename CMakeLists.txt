cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(paladin-engine)

set(PROJECT_SOURCE_FILES
    source/main.cpp
    source/Game.h
    source/Game.cpp
    source/TestObject.h
    source/TestObject.cpp
    source/LabObject.h
    source/LabObject.cpp)

# Add the application
include_directories(source)
include_directories(engine/thirdparty/glew/include)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

# Add engine library
add_compile_definitions(GLEW_NO_GLU)
add_subdirectory(engine "${CMAKE_CURRENT_BINARY_DIR}/engine_build")

include_directories(engine/source)

include_directories(engine/thirdparty/stb_image)
target_link_libraries(${PROJECT_NAME} Engine)

configure_file("scripts/build_config.sh" "${CMAKE_BINARY_DIR}/build_config.sh" @ONLY)

# Doxygen documentation target
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file("${CMAKE_SOURCE_DIR}/Doxyfile.in" "${CMAKE_BINARY_DIR}/Doxyfile" @ONLY)
  add_custom_target(
    engine-docs
    COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating documentation with Doxygen"
    VERBATIM)
endif()
