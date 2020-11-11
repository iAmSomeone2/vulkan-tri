#pragma once

#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <optional>

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
namespace fs = std::filesystem;

namespace VkTri
{
    const fs::path VERTEX_SHADER_PATH("../shaders/vert.spv");
    const fs::path FRAGMENT_SHADER_PATH("../shaders/frag.spv");

    static const uint32_t DISCRETE_SCORE = 1000u;
    static const uint32_t INTEGRATED_SCORE = 500u;
    static const uint32_t VIRTUAL_SCORE = 750u;
    static const uint32_t CPU_ONLY_SCORE = 0u;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool complete() const noexcept
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        vector<vk::SurfaceFormatKHR> formats;
        vector<vk::PresentModeKHR> presentModes;
    };

    class TriangleApp
    {
    private:
        vk::UniqueInstance instance; /**< Application Vulkan instance */
        vk::PhysicalDevice physicalDevice; /**< Physical device in use by the application */
        vk::UniqueDevice logicalDevice; /**< Unique instance of logical Vulkan device. */
        vk::Queue graphicsQueue; /**< Graphics queue used with the logical device. */
        vk::Queue presentQueue; /**< Presentation queue used with the logical device. */

        /**
         * \brief Pointer to the application swap chain
         *
         * \details
         * Giving the instance direct ownership of the UniqueSwapChain object causes the object to
         * be deleted out of sequence. A pointer is used instead to allow Vulkan to manage the object
         * lifetime.
         */
        vk::SwapchainKHR *swapChain;
        vk::UniqueSemaphore imgAvailableSemaphore;
        vk::UniqueSemaphore renderingDoneSemaphore;
        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;
        vector<vk::UniqueImageView> swapChainImageViews;

        vk::UniquePipelineLayout pipelineLayout;
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

        // ============
        // Device setup
        // ============

        const vector<const char *> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        bool checkDeviceExtensionSupport(const vk::PhysicalDevice &device);

        /**
         * \brief Checks the capabilities of the provided device and returns a score.
         * \param device physical Vulkan device to analyze.
         * \return score given to physical Vulkan device.
         */
        uint32_t getDeviceScore(const vk::PhysicalDevice &device);

        /**
         * \brief Decides on which physical device to use.
         */
        void pickPhysicalDevice();

        /**
         * \brief Creates a Vulkan logical device from the provided physical device.
         */
        void createLogicalDevice();

        /**
         * \brief Polls the libraries in-use for the Vulkan extensions they require.
         * \return vector containing the names of the required extensions.
         */
        static vector<const char *> getRequiredExtensions();

        QueueFamilyIndices checkQueueFamilies(const vk::PhysicalDevice &device);

        // =============
        // Surface Setup
        // =============

        vk::UniqueSurfaceKHR surface;

        /**
         * \brief Sets up the application's drawing surface.
         */
        void createSurface();

        // ==========
        // Swap Chain
        // ==========

        SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice &device);

        static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR> &availableFormats);

        static vk::PresentModeKHR chooseSwapPresentMode(const vector<vk::PresentModeKHR> &availableModes);

        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

        void createSwapChain();

        void createSemaphores();

        // =================
        // Graphics Pipeline
        // =================

        void createGraphicsPipeline();

        vk::UniqueShaderModule createShaderModule(const vector<uint8_t> &data);

        // ===========
        // Debug Setup
        // ===========

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
        static constexpr uint32_t WIDTH = 800; /**< Window width */
        static constexpr uint32_t HEIGHT = 600; /**< Window height */

        void run();

        TriangleApp();

        ~TriangleApp();

        static bool checkExtensionSupport(const char **required, const uint32_t &count);

        bool checkValidationLayerSupport();

        [[nodiscard]] static vector<vk::ExtensionProperties> getAvailableExtensions();

        [[nodiscard]] static shared_ptr<TriangleApp> create();

        [[nodiscard]] static vector<uint8_t> readFile(const fs::path &filePath);
    };
}