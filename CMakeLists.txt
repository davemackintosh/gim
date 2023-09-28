cmake_minimum_required(VERSION 3.22.1)
include(FetchContent)
include("./compileGLSL.cmake")

project(
  arteeyes
  VERSION 0.1.0.0
  DESCRIPTION "Gim. I make gim."
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# DEPENDENCIES
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_path(VKB_INCLUDES VkBootstrap.h)

# SOURCES
set(SOURCES ./src/platforms/linux.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
append_glsl_to_target(
  ${PROJECT_SOURCE_DIR}/shaders/voxel.comp
  ${PROJECT_SOURCE_DIR}/shaders/default.frag
  ${PROJECT_SOURCE_DIR}/shaders/default.vert)

include_directories(./include ${VK_INCLUDES})
configure_file(./include/gim/engine.hpp.in ./include/gim/engine.hpp)

# LINKER
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE glm::glm $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
          $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
          Vulkan::Vulkan vk-bootstrap::vk-bootstrap)
