#include <iostream>
#include <map>
#include <set>
#include <array>
#include <algorithm>
#include <fstream>
#include <thread>
#include <cstring>
#include <fmt/format.h>
#include "TriangleApp.hpp"

using std::array;

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

shared_ptr<TriangleApp> TriangleApp::create()
{
    auto triApp = std::make_shared<TriangleApp>();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    triApp->window = glfwCreateWindow(TriangleApp::WIDTH, TriangleApp::HEIGHT, "Vulkan Triangle", nullptr, nullptr);

    triApp->createInstance();
    triApp->setupDebugMessenger();
    triApp->createSurface();
    triApp->pickPhysicalDevice();
    triApp->createLogicalDevice();
    triApp->createSemaphores();
    triApp->createSwapChain();
    triApp->createGraphicsPipeline();

    glfwShowWindow(triApp->window);

    return triApp;
}

void TriangleApp::run()
{
    while (!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();

        // Reduce CPU usage by making the thread sleep.
        // This should be changed to follow the rendering pipeline later.
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    this->cleanup();
}

void TriangleApp::cleanup()
{
    glfwDestroyWindow(this->window);
    this->window = nullptr;
}

vector<uint8_t> TriangleApp::readFile(const fs::path &filePath)
{
    auto fileStream = std::ifstream(filePath, std::ios::ate | std::ios::binary);
    if (!fileStream.is_open())
    {
        throw std::runtime_error(fmt::format("Failed to open file: {:s}", filePath.string()));
    }

    uint32_t fileSize = static_cast<uint32_t>(fileStream.tellg());
    fileStream.seekg(std::ios::beg);
    auto buffer = vector<uint8_t>(fileSize);

    fileStream.read(reinterpret_cast<char *>(buffer.data()), fileSize);
    fileStream.close();

    return buffer;
}

// ===========
// Debug
// ===========

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

// ==============
// Instance
// ==============

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

// ===========
// Surface
// ===========

void TriangleApp::createSurface()
{
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(this->instance.get(), this->window, nullptr, &_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.");
    }

    vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(instance.get());
    this->surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(_surface), _deleter);
}

// ==========
// Swap Chain
// ==========

SwapChainSupportDetails TriangleApp::querySwapChainSupport(const vk::PhysicalDevice &device)
{
    SwapChainSupportDetails details;

    details.capabilities = device.getSurfaceCapabilitiesKHR(this->surface.get());
    details.formats = device.getSurfaceFormatsKHR(this->surface.get());
    details.presentModes = device.getSurfacePresentModesKHR(this->surface.get());

    return details;
}

vk::SurfaceFormatKHR TriangleApp::chooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR> &availableFormats)
{
    vk::SurfaceFormatKHR surfaceFormat;
    for (const auto &format : availableFormats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            surfaceFormat = format;
            break;
        }
        surfaceFormat = format;
    }

    std::clog << fmt::format(FMT_STRING("Chosen Surface Format: {}\n"), surfaceFormat.format);
    return surfaceFormat;
}

vk::PresentModeKHR TriangleApp::chooseSwapPresentMode(const vector<vk::PresentModeKHR> &availableModes)
{
    vk::PresentModeKHR presentMode;
    bool modeFound = false;
    for (const auto &mode : availableModes)
    {
        // The mode type is being checked in order of preference.
        switch (mode)
        {
            case vk::PresentModeKHR::eMailbox:
                presentMode = vk::PresentModeKHR::eMailbox;
                modeFound = true;
                break;
            case vk::PresentModeKHR::eFifoRelaxed:
                presentMode = vk::PresentModeKHR::eFifoRelaxed;
                modeFound = true;
                break;
            case vk::PresentModeKHR::eFifo:
                presentMode = vk::PresentModeKHR::eFifo;
                modeFound = true;
                break;
            default:
                presentMode = mode;
        }

        if (modeFound)
        {
            break;
        }
    }

    std::clog << fmt::format(FMT_STRING("Chosen Swap Present Mode: {}\n"), presentMode);
    return presentMode;
}

vk::Extent2D TriangleApp::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
{
    vk::Extent2D actualExtent;

    int width, height;
    glfwGetFramebufferSize(this->window, &width, &height);

    actualExtent.setWidth(static_cast<uint32_t>(width));
    actualExtent.setHeight(static_cast<uint32_t>(height));

    actualExtent.width = std::max(capabilities.minImageExtent.width,
                                  std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height,
                                   std::min(capabilities.maxImageExtent.height, actualExtent.height));

    std::clog << fmt::format(FMT_STRING("Chosen Swap Extent\tWidth: {:d}px\tHeight: {:d}\n"), actualExtent.width,
                             actualExtent.height);
    return actualExtent;
}

void TriangleApp::createSwapChain()
{
    auto swapChainSupport = this->querySwapChainSupport(this->physicalDevice);

    auto surfaceFormat = this->chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = this->chooseSwapPresentMode(swapChainSupport.presentModes);
    auto extent = this->chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.maxImageCount >= 3 ? 3 : 2;

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = this->surface.get();
    createInfo.minImageCount = imageCount;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1u;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    auto queueIndices = this->checkQueueFamilies(this->physicalDevice);
    array<uint32_t, 2> queueFamilyIndices = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};

    // Deal with the situation in which the required queues are different
    if (queueIndices.graphicsFamily != queueIndices.presentFamily)
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    vk::UniqueSwapchainKHR uniqueSC;
    try
    {
        uniqueSC = logicalDevice->createSwapchainKHRUnique(createInfo);
    }
    catch (const std::runtime_error &err)
    {
        throw err;
    }

    this->swapChain = &uniqueSC.get();

    this->swapChainImageFormat = surfaceFormat.format;
    this->swapChainExtent = extent;
}

void TriangleApp::createSemaphores()
{
    this->imgAvailableSemaphore = this->logicalDevice->createSemaphoreUnique({});
    this->renderingDoneSemaphore = this->logicalDevice->createSemaphoreUnique({});
}

// =================
// Graphics Pipeline
// =================

void TriangleApp::createGraphicsPipeline()
{
    // Set up shader stages
    auto vertShaderData = readFile(VERTEX_SHADER_PATH);
    auto fragShaderData = readFile(FRAGMENT_SHADER_PATH);

    auto vertShaderModule = this->createShaderModule(vertShaderData);
    auto fragShaderModule = this->createShaderModule(fragShaderData);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = vertShaderModule.get();
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = fragShaderModule.get();
    fragShaderStageInfo.pName = "main";

    array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

    // Set up vertex input
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    // Set up input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Set up viewport
    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(this->swapChainExtent.width);
    viewport.height = static_cast<float>(this->swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = swapChainExtent;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Set up rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Set up multisampling
    vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Set up color blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
    colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.logicOpEnable = VK_TRUE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    // Set up pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    this->pipelineLayout = this->logicalDevice->createPipelineLayoutUnique(pipelineLayoutInfo);
}

vk::UniqueShaderModule TriangleApp::createShaderModule(const vector<uint8_t> &data)
{
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = data.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(data.data());

    return this->logicalDevice->createShaderModuleUnique(createInfo);
}

// ============
// Device
// ============

bool TriangleApp::checkDeviceExtensionSupport(const vk::PhysicalDevice &device)
{
    auto availableExts = device.enumerateDeviceExtensionProperties();

    std::set<string> requiredExts(this->deviceExtensions.begin(), this->deviceExtensions.end());

    for (const auto &ext : availableExts)
    {
        requiredExts.erase(ext.extensionName);
    }

    return requiredExts.empty();
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

    // Confirm graphics capability
    auto indices = this->checkQueueFamilies(device);
    if (!indices.complete())
    {
        return 0;
    }

    // Confirm basic swap chain support.
    if (!this->checkDeviceExtensionSupport(device))
    {
        return 0;
    }

    // Confirm the swap chain is adequate
    auto swapChairSupportDetails = this->querySwapChainSupport(device);
    if (swapChairSupportDetails.formats.empty() || swapChairSupportDetails.presentModes.empty())
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
    for (const auto &device : devices)
    {
        score = TriangleApp::getDeviceScore(device);
        suitableDevices.insert(std::make_pair(score, device));
    }

    if (suitableDevices.empty() || suitableDevices.rbegin()->first == 0)
    {
        throw std::runtime_error("Failed to find a suitable GPU.");
    }

    this->physicalDevice = suitableDevices.rbegin()->second;
    std::clog << fmt::format("Selected device: {:s}\n", this->physicalDevice.getProperties().deviceName);
}

void TriangleApp::createLogicalDevice()
{
    auto indices = this->checkQueueFamilies(this->physicalDevice);

    vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    float queuePriority = 1.0f;
    for (auto &queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1u;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    auto deviceFeatures = vk::PhysicalDeviceFeatures();

    auto createInfo = vk::DeviceCreateInfo();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(this->deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = this->deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    try
    {
        this->logicalDevice = this->physicalDevice.createDeviceUnique(createInfo);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
    }

    // Make sure our dispatcher knows about the new device.
    VULKAN_HPP_DEFAULT_DISPATCHER.init(this->logicalDevice.get());

    this->graphicsQueue = this->logicalDevice->getQueue(indices.graphicsFamily.value(), 0);
    this->presentQueue = this->logicalDevice->getQueue(indices.presentFamily.value(), 0);
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

QueueFamilyIndices TriangleApp::checkQueueFamilies(const vk::PhysicalDevice &device)
{
    auto queueFamilies = device.getQueueFamilyProperties();

    QueueFamilyIndices indices;
    uint32_t i = 0;
    for (const auto &fam : queueFamilies)
    {
        // Queue indices may be the same, so check for features in sequence.
        if (fam.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }
        if (device.getSurfaceSupportKHR(i, this->surface.get()))
        {
            indices.presentFamily = i;
        }

        i++;
    }

    return indices;
}

vector<vk::ExtensionProperties> TriangleApp::getAvailableExtensions()
{
    uint32_t count = 0u;
    vk::enumerateInstanceExtensionProperties(nullptr, &count, nullptr, vk::DispatchLoaderStatic());

    vector<vk::ExtensionProperties> extensions(count);
    vk::enumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

    return extensions;
}
