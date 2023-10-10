#include "Instance.hpp"
#include "Log.hpp"
#include <vulkan/vulkan_core.h>
#include <algorithm>

VulkanInstance::VulkanInstance(const Window& window)
{
    VkResult result;

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Untitled Game",
        .applicationVersion = 1,
        .pEngineName = nullptr,
        .apiVersion = VK_API_VERSION_1_3};

    std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    std::vector<const char*> extNames = GetExtensionNames(window);

    VkInstanceCreateFlags instanceCreateFlags =
        GetInstanceCreateFlags(extNames);

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = instanceCreateFlags,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extNames.size()),
        .ppEnabledExtensionNames = extNames.data()};

    result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
    LogVulkanError("Could not create vulkan instance", result);
}

VkInstanceCreateFlags VulkanInstance::GetInstanceCreateFlags(
    const std::vector<const char*>& extentionNames)
{
    VkInstanceCreateFlags instanceCreateFlags = {};
    if (std::find_if(
            extentionNames.begin(), extentionNames.end(),
            [](const char* extName) {
                return !strcmp(
                    extName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            }) != extentionNames.end())
    {
        instanceCreateFlags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    return instanceCreateFlags;
}

std::vector<const char*> VulkanInstance::GetExtensionNames(const Window& window)
{
    std::vector<const char*> extNames = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

    std::vector<const char*> windowExtNames =
        window.GetRequiredExtensionNames();
    extNames.insert(
        extNames.end(), windowExtNames.begin(), windowExtNames.end());

    std::vector<VkExtensionProperties> instanceSupportedExtensions =
        GetInstanceSupportedExtensions();

    if (std::find_if(
            instanceSupportedExtensions.begin(),
            instanceSupportedExtensions.end(),
            [](VkExtensionProperties& properties)
            {
                return !strcmp(
                    properties.extensionName,
                    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            }) != instanceSupportedExtensions.end())
    {
        extNames.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    }

    for (const char* extName : extNames)
    {
        if (std::find_if(
                instanceSupportedExtensions.begin(),
                instanceSupportedExtensions.end(),
                [extName](VkExtensionProperties& properties) {
                    return !strcmp(properties.extensionName, extName);
                }) == instanceSupportedExtensions.end())
        {
            LogError(
                fmt::format("Required extension {} not supported!", extName));
        }
    }

    LogDebug("Enabled exensions:");
    for (const char* extName : extNames)
    {
        LogDebug(fmt::format("\t{}", extName));
    }
    return extNames;
}

std::vector<VkExtensionProperties>
VulkanInstance::GetInstanceSupportedExtensions()
{
    VkResult result;

    uint32_t instanceSupportedExtensionCount;
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &instanceSupportedExtensionCount, nullptr);
    LogVulkanError("Failed to get instance supported extension count", result);

    std::vector<VkExtensionProperties> instanceSupportedExtensions(
        instanceSupportedExtensionCount);
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &instanceSupportedExtensionCount,
        instanceSupportedExtensions.data());
    LogVulkanError("Failed to get instance supported extension count", result);

    LogDebug("Instance Supported Extensions:");
    for (const VkExtensionProperties& properties : instanceSupportedExtensions)
    {
        LogDebug(fmt::format("\t{}", properties.extensionName));
    }
    return instanceSupportedExtensions;
}