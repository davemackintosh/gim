cmake_minimum_required(VERSION 3.22.1)

project(
  walk-to-utopia
  VERSION 0.1.0
  DESCRIPTION "Gim. I test gim."
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# DEPENDENCIES
find_package(doctest REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_path(VULKAN_HPP_INCLUDE_DIRS "vulkan/vulkan.hpp")

# SOURCES
set(SOURCES ./test_main.cpp)
file(
  GLOB_RECURSE HEADERS
  RELATIVE ${PROJECT_SOURCE_DIR}
  "include/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES})
append_glsl_to_target(
  ${PROJECT_SOURCE_DIR}/shaders/voxel.comp
  ${PROJECT_SOURCE_DIR}/shaders/default.frag
  ${PROJECT_SOURCE_DIR}/shaders/default.vert)

include_directories(./include)

# LINKER
target_include_directories(${PROJECT_NAME} PRIVATE ${VULKAN_HPP_INCLUDE_DIRS}
                                                   ${Vulkan_INCLUDE_DIRS})
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE glm::glm $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
          $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
          Vulkan::Vulkan)
