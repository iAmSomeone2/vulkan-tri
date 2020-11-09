#include <iostream>
#include <map>
#include <cstring>
#include <fmt/format.h>
#include "TriangleApp.hpp"

using namespace VkTri;

TriangleApp::TriangleApp()
{
    this->window = nullptr;
};

TriangleApp::~TriangleApp()
{
    if (this->window != nullptr)
    {
        this->cleanup();
    }
}

void TriangleApp::cleanup()
{
    glfwDestroyWindow(this->window);
    this->window = nullptr;
}

void TriangleApp::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    auto messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    auto messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

    // The result of this does not need to be stored!
    this->instance->createDebugUtilsMessengerEXTUnique(
            vk::DebugUtilsMessengerCreateInfoEXT({}, messageSeverity, messageType,
                                                 reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(TriangleApp::debugCallback),
                                                 nullptr));

}

vk::Bool32 TriangleApp::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                      vk::DebugUtilsMessageTypeFlagBitsEXT type,
                                      const vk::DebugUtilsMessengerCallbackDataEXT *callbackData, void *userData)
{
    if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    {
        switch (type)
        {
            case vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral:
                std::clog << fmt::format("General error: {:s}\n", callbackData->pMessage);
                break;
            case vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation:
                std::clog << fmt::format("Validation layer error: {:s}\n", callbackData->pMessage);
                break;
            case vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance:
                std::clog << fmt::format("Performance error: {:s}\n", callbackData->pMessage);
                break;
        }
    }
    return VK_FALSE;
}

vector<const char *> TriangleApp::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0u;
    const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    vector<const char *> extensions;
    extensions.reserve(glfwExtensionCount);
    for (uint32_t i = 0u; i < glfwExtensionCount; ++i)
    {
        extensions.push_back(glfwExtensions[i]);
    }

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void TriangleApp::createInstance()
{
    // Set up dynamic extension loading.
    vk::DynamicLoader dynaLoader;
    auto vkGetInstanceProcAddr = dynaLoader.getProcAddress<PFN_vkGetInstanceProcAddr>(
            "vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    // Check for Validation Layers if they are in use
    if (enableValidationLayers && !this->checkValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers requested but not available.");
    }

    // Set up app info
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "Vulkan Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Set up instance
    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;

    // Set up required extensions
    auto requiredExts = TriangleApp::getRequiredExtensions();

    if (!TriangleApp::checkExtensionSupport(requiredExts.data(), requiredExts.size()))
    {
        throw std::runtime_error("Missing required Vulkan extensions.");
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExts.size());
    createInfo.ppEnabledExtensionNames = requiredExts.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0u;
    }

    // Add instance to the dispatch loader.
    this->instance = vk::createInstanceUnique(createInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*this->instance);
}

uint32_t TriangleApp::getDeviceScore(const vk::PhysicalDevice &device)
{
    uint32_t score = 0u;

    auto properties = device.getProperties();
    auto features = device.getFeatures();

    // Check device type
    switch (properties.deviceType)
    {
        case vk::PhysicalDeviceType::eOther:
            // Don't score the device
            break;
        case vk::PhysicalDeviceType::eIntegratedGpu:
            score += INTEGRATED_SCORE;
            break;
        case vk::PhysicalDeviceType::eDiscreteGpu:
            score += DISCRETE_SCORE;
            break;
        case vk::PhysicalDeviceType::eVirtualGpu:
            score += VIRTUAL_SCORE;
            break;
        case vk::PhysicalDeviceType::eCpu:
            score += CPU_ONLY_SCORE;
            break;
    }

    // Get max texture size;
    score += properties.limits.maxImageDimension2D;

    // Get dedicated VRAM
    score += properties.limits.maxMemoryAllocationCount;

    // Confirm geometry shader presence.
    if (!features.geometryShader)
    {
        return 0;
    }

    return score;
}

void TriangleApp::pickPhysicalDevice()
{
    auto devices = this->instance->enumeratePhysicalDevices();
    if (devices.empty())
    {
        throw std::runtime_error("Failed to find any GPUs with Vulkan support.");
    }

    std::multimap<uint32_t, vk::PhysicalDevice> suitableDevices;
    uint32_t score;
    for(const auto &device : devices)
    {
        score = TriangleApp::getDeviceScore(device);
        suitableDevices.insert(std::make_pair(score, device));
    }

    if (suitableDevices.empty() || suitableDevices.rbegin()->first == 0)
    {
        throw std::runtime_error("Failed to find a suitable GPU.");
    }

    this->physicalDevice = suitableDevices.rbegin()->second;
}

bool TriangleApp::checkExtensionSupport(const char **required, const uint32_t &count)
{
    vector<std::string> requiredExts;
    requiredExts.reserve(count);
    for (uint32_t i = 0u; i < count; ++i)
    {
        requiredExts.emplace_back(required[i]);
    }

    const auto availableExtensions = TriangleApp::getAvailableExtensions();

    vector<string> missing;
    for (const auto &ext : requiredExts)
    {
        bool found = false;
        for (const auto &avail : availableExtensions)
        {
            if (ext == avail.extensionName)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            missing.push_back(ext);
        }
    }

    if (missing.empty())
    {
        return true;
    }

    std::cerr << "Missing extension(s):\n";
    for (const auto &ext : missing)
    {
        std::clog << "\t" << ext << "\n";
    }

    return false;
}

bool TriangleApp::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vk::enumerateInstanceLayerProperties(&layerCount, nullptr, vk::DispatchLoaderStatic());

    vector<vk::LayerProperties> availableLayers(layerCount);
    vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // Check against required layers
    vector<std::string> missing;
    for (const auto layer : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProps : availableLayers)
        {
            if (strncmp(layer, layerProps.layerName, 255) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            missing.emplace_back(layer);
        }
    }

    if (missing.empty())
    {
        return true;
    }

    std::cerr << "Missing layers(s):\n";
    for (const auto &ext : missing)
    {
        std::clog << "\t" << ext << "\n";
    }

    return false;
}

vector<vk::ExtensionProperties> TriangleApp::getAvailableExtensions()
{
    uint32_t count = 0u;
    vk::enumerateInstanceExtensionProperties(nullptr, &count, nullptr, vk::DispatchLoaderStatic());

    vector<vk::ExtensionProperties> extensions(count);
    vk::enumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

    return extensions;
}

shared_ptr<TriangleApp> TriangleApp::create()
{
    auto triApp = std::make_shared<TriangleApp>();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    triApp->window = glfwCreateWindow(TriangleApp::WIDTH, TriangleApp::HEIGHT, "Vulkan Triangle", nullptr, nullptr);

    triApp->createInstance();
    triApp->setupDebugMessenger();
    triApp->pickPhysicalDevice();

    return triApp;
}

void TriangleApp::run()
{
    while (!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();
    }
    this->cleanup();
}
