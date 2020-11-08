# Begin GLFW 3 Configuration

set(BUILD_SHARED_LIBS off)
set(GLFW_BUILD_EXAMPLES off)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set(GLFW_BUILD_TESTS on)
else()
    set(GLFW_BUILD_TESTS off)
endif()

set(GLFW_BUILD_DOCS off)
set(GLFW_VULKAN_STATIC off)

add_subdirectory(glfw)

# END GLFW 3 Configuration
