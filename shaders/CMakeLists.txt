find_program(GLSLC glslc HINTS
    /usr/bin/glslc /usr/local/bin/glslc
    REQUIRED)

# Compile vertex shaders 

set(VERTEX_SHADERS triangle.vert)

set(COMPILED_VERTEX_SHADERS ${CMAKE_CURRENT_BINARY_DIR}/vert.spv)
set_source_files_properties(${COMPILED_VERTEX_SHADERS} PROPERTIES GENERATED TRUE)

add_custom_command(OUTPUT ${COMPILED_VERTEX_SHADERS}
        PRE_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${GLSLC}
        ARGS ${VERTEX_SHADERS} -o ${COMPILED_VERTEX_SHADERS}
        DEPENDS ${VERTEX_SHADERS}
        COMMENT "Compiling vertex shaders..."
        )

add_custom_target(vertex_shaders
        DEPENDS ${COMPILED_VERTEX_SHADERS}
        )

# Compile fragment shaders

set(FRAGMENT_SHADERS triangle.frag)

set(COMPILED_FRAGMENT_SHADERS ${CMAKE_CURRENT_BINARY_DIR}/frag.spv)
set_source_files_properties(${COMPILED_FRAGMENT_SHADERS} PROPERTIES GENERATED TRUE)

add_custom_command(OUTPUT ${COMPILED_FRAGMENT_SHADERS}
        PRE_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${GLSLC}
        ARGS ${FRAGMENT_SHADERS} -o ${COMPILED_FRAGMENT_SHADERS}
        DEPENDS ${FRAGMENT_SHADERS}
        COMMENT "Compiling fragment shaders..."
        )

add_custom_target(fragment_shaders
        DEPENDS ${COMPILED_FRAGMENT_SHADERS}
        )

add_custom_target(vulkan_shaders ALL
    DEPENDS fragment_shaders vertex_shaders)


