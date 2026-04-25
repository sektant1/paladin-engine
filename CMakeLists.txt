cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(coagula-engine)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

set(PROJECT_SOURCE_FILES source/main.cpp source/Game.h source/Game.cpp source/Player.h source/Player.cpp source/Bullet.h source/Bullet.cpp)
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

# Symlink compile_commands.json into the repo root so clangd / IDEs find it
# regardless of which machine or build directory (bin/debug, bin/release) is used.
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink
          "${CMAKE_BINARY_DIR}/compile_commands.json"
          "${CMAKE_SOURCE_DIR}/compile_commands.json"
  VERBATIM)

configure_file("scripts/build_config.sh" "${CMAKE_BINARY_DIR}/build_config.sh"
               @ONLY)

# Doxygen documentation target
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file("${CMAKE_SOURCE_DIR}/Doxyfile.in"
                 "${CMAKE_BINARY_DIR}/Doxyfile" @ONLY)
  add_custom_target(
    engine-docs
    COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating documentation with Doxygen"
    VERBATIM)
endif()
