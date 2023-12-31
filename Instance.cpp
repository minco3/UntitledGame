#include "Instance.hpp"
#include "Log.hpp"
#include <algorithm>
#include <vulkan/vulkan_core.h>

VulkanInstance::VulkanInstance(const Window& window, vk::raii::Context& context)
    : m_Instance(CreateInstance(window, context))
{
}

vk::raii::Instance
VulkanInstance::CreateInstance(const Window& window, vk::raii::Context& context)
{
    vk::ApplicationInfo applicationInfo("Untitled Game", 1);

    std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    std::vector<const char*> extNames = GetExtensionNames(window);

    vk::InstanceCreateFlags instanceCreateFlags =
        GetInstanceCreateFlags(extNames);

    vk::InstanceCreateInfo createInfo(instanceCreateFlags, &applicationInfo, instanceLayers, extNames);

    return context.createInstance(createInfo);
}

vk::InstanceCreateFlags VulkanInstance::GetInstanceCreateFlags(
    const std::vector<const char*>& extentionNames)
{
    vk::InstanceCreateFlags instanceCreateFlags = {};
    if (std::find_if(
            extentionNames.begin(), extentionNames.end(),
            [](const char* extName) {
                return !strcmp(
                    extName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            }) != extentionNames.end())
    {
        instanceCreateFlags |=
            vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
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

    std::vector<vk::ExtensionProperties> instanceSupportedExtensions =
        vk::enumerateInstanceExtensionProperties();

    if (std::find_if(
            instanceSupportedExtensions.begin(),
            instanceSupportedExtensions.end(),
            [](vk::ExtensionProperties& properties)
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
                [extName](vk::ExtensionProperties& properties) {
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