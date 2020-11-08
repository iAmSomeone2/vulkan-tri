#define GLFW_INCLUDE_VULKAN
extern "C"
{
#include <glfw3.h>
}

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vulkan/vulkan.hpp>

#include <fmt/format.h>

#include <iostream>
#include <memory>

using std::unique_ptr;

void errorCallback(int error, const char* desc)
{
    std::cerr << fmt::format(FMT_STRING("Error {:d}: {:s}\n"), error, desc);
}

int main()
{
    // Init the library
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW.\n";
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(errorCallback);
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    assert(monitor != nullptr);
    GLFWwindow *window;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(800, 600, "Vulkan Test", monitor, nullptr);
    if(window == nullptr)
    {
        std::cerr << "Failed tp create GLFW window.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    uint32_t extensionCount = 0u;
    auto result = vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr, vk::DispatchLoaderStatic());

    if (result != vk::Result::eSuccess)
    {
        std::cerr << "Failed to get Vulkan extensions.\n";
        glfwDestroyWindow(window);
        return EXIT_FAILURE;
    }

    std::clog << fmt::format(FMT_STRING("{:d} extensions supported.\n"), extensionCount);

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    // while(!glfwWindowShouldClose(window))
    // {
    //     glfwPollEvents();
    // }

    glfwDestroyWindow(window);

    glfwTerminate();
    return EXIT_SUCCESS;
}
