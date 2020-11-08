add_executable(VulkanTest vk_test.cpp)

target_include_directories(VulkanTest PUBLIC SYSTEM
    ${glfw_INCLUDES}
    ${Vulkan_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS})

target_link_libraries(VulkanTest
    ${Vulkan_LIBRARIES}
    glfw
    ${GLM_LIBRARIES}
    fmt::fmt)

target_compile_features(VulkanTest PUBLIC
    cxx_auto_type)

add_test(NAME BasicTest COMMAND VulkanTest)
