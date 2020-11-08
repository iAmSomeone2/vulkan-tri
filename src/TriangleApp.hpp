#pragma once

#include <memory>
#include <vector>

#define GLFW_INCLUDE_VULKAN
extern "C"
{
#include <glfw3.h>
}

#include <vulkan/vulkan.hpp>

using std::vector;
using std::shared_ptr;

namespace VkTri
{
    class TriangleApp {
    protected:
        // Validation layers
#ifdef NDEBUG
        static constexpr bool enableValidationLayers = false;
#else
        static constexpr bool enableValidationLayers = true;
        const vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };
#endif // NDEBUG

        GLFWwindow *window; /**< Application window */
        vk::Instance instance; /**< Application Vulkan instance */

        void cleanup();

        void createInstance();
    public:
        static constexpr uint32_t WIDTH = 800;
        static constexpr uint32_t HEIGHT = 600;

        void run();

        TriangleApp();

        ~TriangleApp();

        static bool checkExtensionSupport(const char **required, const uint32_t &count);

        static vector<vk::ExtensionProperties> getAvailableExtensions();

        static shared_ptr<TriangleApp> create();
    };
}
