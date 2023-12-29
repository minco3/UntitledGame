#include "Device.hpp"
#include "vulkan/vulkan_beta.h"
#include <string_view>

Device::Device(vk::raii::Instance& instance, Surface& surface)
    : m_PhysicalDevice(instance.enumeratePhysicalDevices().front()),
      m_Device(m_PhysicalDevice, GetDeviceCreateInfo(surface))
{
}

vk::DeviceCreateInfo Device::GetDeviceCreateInfo(Surface& surface)
{
    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos =
        GetDeviceQueueCreateInfos(surface);
    std::vector<const char*> deviceExtensions = GetDeviceExtentionNames();

    std::vector<const char*> deviceLayers;

    return vk::DeviceCreateInfo(
        vk::DeviceCreateFlags(), deviceQueueCreateInfos, deviceLayers,
        deviceExtensions);
}

std::vector<vk::DeviceQueueCreateInfo>
Device::GetDeviceQueueCreateInfos(Surface& surface)
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
        m_PhysicalDevice.getQueueFamilyProperties();

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    deviceQueueCreateInfos.reserve(queueFamilyProperties.size());

    m_DeviceQueues.resize(
        queueFamilyProperties.size(), {{std::vector<float>{1.0f}}});

    LogDebug("Queues:");
    for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
    {
        const vk::QueueFamilyProperties& properties = queueFamilyProperties.at(i);

        m_DeviceQueues.at(i).m_PresentationSupported =
            m_PhysicalDevice.getSurfaceSupportKHR(i, *surface.Get());

        //TODO: fix queue flags not being able to be print

        LogDebug(fmt::format(
            "\tQueue Family [{}]: {:#b} {}", i,
            properties.queueCount,
            static_cast<bool>(m_DeviceQueues.at(i).m_PresentationSupported)));

        deviceQueueCreateInfos.emplace_back(
            0, i, m_DeviceQueues.at(i).m_Priorities);
    }
    return deviceQueueCreateInfos;
}

std::vector<const char*> Device::GetDeviceExtentionNames()
{
    VkResult result;
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<vk::ExtensionProperties> deviceSupportedExtensions =
        m_PhysicalDevice.enumerateDeviceExtensionProperties();

    LogDebug("Physical Device Supported Extensions:");
    for (const auto properties : deviceSupportedExtensions)
    {
        LogDebug(
            fmt::format("\t{}", std::string_view(properties.extensionName)));
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

vk::raii::Device& Device::Get() { return m_Device; }

uint32_t Device::FindMemoryType(
    vk::MemoryRequirements memoryRequirements,
    vk::MemoryPropertyFlags memoryPropertyFlags)
{

    vk::PhysicalDeviceMemoryProperties memoryProperties =
        m_PhysicalDevice.getMemoryProperties();

    uint32_t memoryTypeIndex;

    LogDebug(
        fmt::format("TypeFilter: {:#b}", memoryRequirements.memoryTypeBits));

    LogDebug(fmt::format("Available Memory Types:"));
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        LogDebug(fmt::format(
            "\t{:#b}", static_cast<uint32_t>(
                           memoryProperties.memoryTypes[i].propertyFlags)));
    }

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags &
             memoryPropertyFlags) == memoryPropertyFlags)
        {
            memoryTypeIndex = i;
            break;
        }
        if (i == memoryProperties.memoryTypeCount - 1)
        {
            LogError("ERROR: failed to find suitable memory type\n");
        }
    }
    return memoryTypeIndex;
}

vk::SurfaceCapabilitiesKHR
Device::GetSurfaceCapabilities(vk::raii::SurfaceKHR& surface)
{
    return m_PhysicalDevice.getSurfaceCapabilitiesKHR(*surface);
}
std::vector<vk::SurfaceFormatKHR>
Device::GetCompatableSurfaceFormats(vk::raii::SurfaceKHR& surface)
{
    std::vector<vk::SurfaceFormatKHR> surfaceFormats =
        m_PhysicalDevice.getSurfaceFormatsKHR(*surface);

    LogDebug("Supported Formats:");
    for (const auto& surfaceFormat : surfaceFormats)
    {
        LogDebug(fmt::format(
            "\t{}, {}", surfaceFormat.colorSpace, surfaceFormat.format));
    }
    return surfaceFormats;
}