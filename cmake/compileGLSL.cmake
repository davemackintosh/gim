macro(append_glsl_to_target)
  message("Processing shaders: ${ARGV} to ${PROJECT_BINARY_DIR}/shaders/")
  foreach(GLSL ${ARGV})
    message("Processing: ${GLSL}")
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")

    message("VALIDATOR", Vulkan::glslc)
    add_custom_command(
      OUTPUT ${SPIRV}
      COMMAND cd
      COMMAND echo "building shader: '${SPIRV}'"
      COMMAND "${CMAKE_COMMAND}" -E make_directory
              "${PROJECT_BINARY_DIR}/shaders/"
      COMMAND Vulkan::glslc "${GLSL}" "-o" "${SPIRV}"
      BYPRODUCTS ${SPIRV}
      DEPENDS ${GLSL})

    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
  endforeach(GLSL)

  add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

  add_dependencies(${PROJECT_NAME} Shaders)
endmacro()
