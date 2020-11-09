#pragma once

#include <memory>
#include <vector>
#include <string>

#define GLFW_INCLUDE_VULKAN
extern "C"
{
#include <glfw3.h>
}

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
using std::string;
using std::vector;
using std::shared_ptr;

namespace VkTri
{
    const uint32_t DISCRETE_SCORE = 1000u;
    const uint32_t INTEGRATED_SCORE = 500u;
    const uint32_t VIRTUAL_SCORE = 750u;
    const uint32_t CPU_ONLY_SCORE = 0u;

    class TriangleApp
    {
    private:
        vk::UniqueInstance instance; /**< Application Vulkan instance */
        vk::PhysicalDevice physicalDevice; /**< Physical device in use by the application */
    protected:
        // Validation layers
        const vector<const char *> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };
#ifdef NDEBUG
        static constexpr bool enableValidationLayers = false;
#else
        static constexpr bool enableValidationLayers = true;
#endif // NDEBUG

        GLFWwindow *window; /**< Application window */

        /**
         * \brief Deconstructs all of the Vulkan data and structures in the app instance.
         *
         * \details
         * This is not a part of the destructor because GLFW terminates before the destructor is called.
         */
        void cleanup();

        /**
         * \brief Sets up the Vulkan instance attached to this app instance.
         */
        void createInstance();

        static uint32_t getDeviceScore(const vk::PhysicalDevice &device);

        void pickPhysicalDevice();

        /**
         * \brief Polls the libraries in-use for the Vulkan extensions they require.
         * \return vector containing the names of the required extensions.
         */
        static vector<const char *> getRequiredExtensions();

        void setupDebugMessenger();

        /**
         * \brief Used to assist with Vulkan debugging.
         * \param severity
         * \param type
         * \param callbackData
         * \param userData
         * \return
         */
        static VKAPI_ATTR vk::Bool32 VKAPI_CALL
        debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagBitsEXT type,
                      const vk::DebugUtilsMessengerCallbackDataEXT *callbackData, void *userData);

    public:
        static constexpr uint32_t WIDTH = 800;
        static constexpr uint32_t HEIGHT = 600;

        void run();

        TriangleApp();

        ~TriangleApp();

        static bool checkExtensionSupport(const char **required, const uint32_t &count);

        bool checkValidationLayerSupport();

        static vector<vk::ExtensionProperties> getAvailableExtensions();

        static shared_ptr<TriangleApp> create();
    };
}
