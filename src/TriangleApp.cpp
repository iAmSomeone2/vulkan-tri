#include <iostream>
#include <string>
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
    this->instance.destroy();
    glfwDestroyWindow(this->window);
    this->window = nullptr;
}

void TriangleApp::createInstance()
{
    // Set up app info
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "Vulkan Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;

    // Set up required extensions
    uint32_t glfwExtensionCount = 0u;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    if(!TriangleApp::checkExtensionSupport(glfwExtensions, glfwExtensionCount))
    {
        throw std::runtime_error("Missing required Vulkan extensions.");
    }

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0u;

    if(vk::createInstance(&createInfo, nullptr, &this->instance) != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create Vulkan instance.");
    }
}

bool TriangleApp::checkExtensionSupport(const char **required, const uint32_t &count)
{
    vector<std::string> requiredExts;
    requiredExts.reserve(count);
    for(uint32_t i = 0u; i < count; ++i)
    {
        requiredExts.emplace_back(required[i]);
    }

    const auto availableExtensions = TriangleApp::getAvailableExtensions();

    vector<std::string> missing;
    for (const auto &ext : requiredExts)
    {
        bool found = false;
        for (const auto &avail : availableExtensions)
        {
            if(ext == avail.extensionName)
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

    return triApp;
}

void TriangleApp::run()
{
    while(!glfwWindowShouldClose(this->window))
    {
        glfwPollEvents();
    }
    this->cleanup();
}
