cmake_minimum_required(VERSION 3.22.1)

project(
  arteeyes
  VERSION 0.1.0 # https://semver.org/spec/v0.1.0.html
  DESCRIPTION "Gim. I make gim."
  # HOMEPAGE_URL ""
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# SOURCES
file(
  GLOB_RECURSE SOURCES
  RELATIVE ${PROJECT_SOURCE_DIR}
  "src/*.cpp")
file(
  GLOB_RECURSE HEADERS
  RELATIVE ${PROJECT_SOURCE_DIR}
  "include/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# DEPENDENCIES
find_package(glm REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_path(VULKAN_HPP_INCLUDE_DIRS "vulkan/vulkan.hpp")

# LINKER
target_include_directories(${PROJECT_NAME} PRIVATE ${VULKAN_HPP_INCLUDE_DIRS})
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE glm::glm $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
          $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
