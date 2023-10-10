#include "Device.hpp"
#include "vulkan/vulkan_beta.h"

Device::Device(VkInstance& instance, Surface surface)
{
    VkResult result;
    std::vector<VkPhysicalDevice> physicalDevices = GetPhysicalDevices(instance);
    m_PhysicalDevice = physicalDevices[0];
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos =
        GetDeviceQueueCreateInfos(surface);
    std::vector<const char*> deviceExtensions =
        GetDeviceExtentionNames();

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount =
            static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        .pQueueCreateInfos = deviceQueueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data()};

    result =
        vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
    LogVulkanError("Could not create vulkan logical device", result);
}

std::vector<VkPhysicalDevice> Device::GetPhysicalDevices(VkInstance instance)
{
    VkResult result;
    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        LogError("No vukan devices found!");
    }
    LogVulkanError("Could not fetch physical device count", result);

    std::vector<VkPhysicalDevice> physicalDevices(
        static_cast<size_t>(deviceCount));
    result = vkEnumeratePhysicalDevices(
        instance, &deviceCount, &physicalDevices.front());
    LogVulkanError("Problem enumerating physical devices", result);

    LogDebug("Physical Devices:");
    for (uint32_t i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices.at(i), &properties);
        LogDebug(fmt::format(
            "\tPhysical device [{}]: {}", i, properties.deviceName));
    }

    return physicalDevices;
}

std::vector<VkDeviceQueueCreateInfo> Device::GetDeviceQueueCreateInfos(Surface surface)
{
    VkResult result;
    uint32_t queueFamilyPropertyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(
        m_PhysicalDevice, &queueFamilyPropertyCount, nullptr);
    if (queueFamilyPropertyCount == 0)
    {
        LogError("No physical device queues found");
    }
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        m_PhysicalDevice, &queueFamilyPropertyCount,
        queueFamilyProperties.data());

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos(
        static_cast<size_t>(queueFamilyPropertyCount));

    m_DeviceQueues.resize(
        queueFamilyPropertyCount, {{std::vector<float>{1.0f}}});

    LogDebug("Queues:");
    for (uint32_t i = 0; i < queueFamilyPropertyCount; i++)
    {
        const VkQueueFamilyProperties& properties = queueFamilyProperties.at(i);

        result = vkGetPhysicalDeviceSurfaceSupportKHR(
            m_PhysicalDevice, i, surface(),
            &m_DeviceQueues.at(i).m_PresentationSupported);
        LogVulkanError(
            fmt::format(
                "Failed to check whether queue {} supports presentation", i),
            result);

        LogDebug(fmt::format(
            "\tQueue Family [{}]: {:#b} {} {}", i, properties.queueFlags,
            properties.queueCount,
            static_cast<bool>(m_DeviceQueues.at(i).m_PresentationSupported)));

        VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = i,
            .queueCount = 1,
            .pQueuePriorities = m_DeviceQueues.at(i).m_Priorities.data()};
        deviceQueueCreateInfos.at(i) = (deviceQueueCreateInfo);
    }
    return deviceQueueCreateInfos;
}

std::vector<const char*> Device::GetDeviceExtentionNames()
{
VkResult result;
    uint32_t deviceSupportedExtensionsCount = 0;
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    result = vkEnumerateDeviceExtensionProperties(
        m_PhysicalDevice, nullptr, &deviceSupportedExtensionsCount, nullptr);
    LogVulkanError("error getting available device extension count", result);

    std::vector<VkExtensionProperties> deviceSupportedExtensions(
        static_cast<size_t>(deviceSupportedExtensionsCount));
    result = vkEnumerateDeviceExtensionProperties(
        m_PhysicalDevice, nullptr, &deviceSupportedExtensionsCount,
        deviceSupportedExtensions.data());
    LogVulkanError("error getting available device extensions", result);

    LogDebug("Physical Device Supported Extensions:");
    for (const auto properties : deviceSupportedExtensions)
    {
        LogDebug(fmt::format("\t{}", properties.extensionName));
    }

    for (const auto& extension : deviceSupportedExtensions)
    {
        // https://vulkan.lunarg.com/doc/view/1.3.250.1/mac/1.3-extensions/vkspec.html#VUID-VkDeviceCreateInfo-pProperties-04451
        if (!strcmp(
                extension.extensionName,
                VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
        {
            deviceExtensions.push_back(
                VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
        }
    }

    return deviceExtensions;
}

VkDevice& Device::operator()()
{
    return m_Device;
}
