set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}bin/glslangValidator")

macro(append_glsl_to_target)
  message("Processing shaders: ${ARGV}")
  foreach(GLSL ${ARGV})
    message("Processing: ${GLSL}")
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")

    add_custom_command(
      OUTPUT ${SPIRV}
      COMMAND cd
      COMMAND "${CMAKE_COMMAND}" -E make_directory
              "${PROJECT_BINARY_DIR}/shaders/"
      COMMAND "${GLSL_VALIDATOR}" -V ${GLSL} -o ${SPIRV}
      DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
  endforeach(GLSL)

  add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

  add_dependencies(${PROJECT_NAME} Shaders)

  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_BINARY_DIR}/shaders"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders")
endmacro()
