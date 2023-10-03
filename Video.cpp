#include "Video.hpp"
#include "Log.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <algorithm>
#include <fmt/format.h>
#include <vulkan/vulkan_beta.h>

Video::Video()
    : m_Window(
          "Untitled Game", {SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED},
          {1200, 800}, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN)
{
    CreateInstance();
}
Video::~Video() { vkDestroyInstance(m_Instance, nullptr); }

void Video::CreateInstance()
{
    VkResult result;

    VkInstanceCreateFlags instanceCreateFlags = {};

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Untitled Game",
        .applicationVersion = 1,
        .pEngineName = nullptr,
        .apiVersion = VK_API_VERSION_1_3};

    std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    std::vector<const char*> extNames = GetExtensionNames();

    if (std::find_if(
            extNames.begin(), extNames.end(),
            [](const char* extName) {
                return !strcmp(
                    extName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            }) != extNames.end())
    {
        instanceCreateFlags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

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

void Video::CreateDevice()
{
    VkResult result;
    std::vector<VkPhysicalDevice> physicalDevices = GetPhysicalDevices();
    m_PhysicalDevice = physicalDevices[0];
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos =
        GetDeviceQueueCreateInfos(m_PhysicalDevice);
    std::vector<const char*> deviceExtensions =
        GetDeviceExtentionNames(m_PhysicalDevice);

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

void Video::CreateSwapchain()
{
    VkResult result;
    m_SurfaceFormat = GetSurfaceFormat();
    VkSurfaceCapabilitiesKHR surfaceCapabilities = GetSurfaceCapabilities();
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_Surface,
        .minImageCount = surfaceCapabilities.minImageCount + 1,
        .imageFormat = m_SurfaceFormat.format,
        .imageColorSpace = m_SurfaceFormat.colorSpace,
        .imageExtent = surfaceCapabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = true};

    VkSwapchainKHR swapchain;
    result = vkCreateSwapchainKHR(
        m_Device, &swapchainCreateInfo, nullptr, &swapchain);
    LogVulkanError("Failed to create swapchain", result);
}

void Video::CreateSwapchainImageViews()
{
    VkResult result;
    std::vector<VkImage> swapchainImages = GetSwapchainImages();
    std::vector<VkImageView> swapchainImageViews(swapchainImages.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++)
    {
        VkImageView& swapchainImageView = swapchainImageViews.at(i);
        VkImageViewCreateInfo swapchainImageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchainImages.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_SurfaceFormat.format,
            .components =
                {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                 .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                 .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                 .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1}};
        result = vkCreateImageView(
            m_Device, &swapchainImageViewCreateInfo, nullptr,
            &swapchainImageView);
        LogVulkanError("Failed to create swapchain image view", result);
    }
}

void Video::LoadShaders()
{
    VkResult result;
    for (Shader& shader : m_ShaderModules)
    {
        std::ifstream shaderCodeFile(shader.filePath, std::ios::binary);
        if (!shaderCodeFile.is_open())
        {
            LogWarning(fmt::format(
                "Requested shader file {} missing!", shader.filePath));
        }
        std::string shaderCode(
            (std::istreambuf_iterator<char>(shaderCodeFile)),
            std::istreambuf_iterator<char>());

        VkShaderModuleCreateInfo vertShaderCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = shaderCode.size(),
            .pCode = reinterpret_cast<const uint32_t*>(shaderCode.data())};

        result = vkCreateShaderModule(
            m_Device, &vertShaderCreateInfo, nullptr, &shader.shaderModule);
        LogVulkanError(
            fmt::format("failed to create shader {}", shader.filePath), result);
    }
}

std::vector<const char*> Video::GetExtensionNames()
{
    std::vector<const char*> extNames = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

    std::vector<const char*> windowExtNames =
        m_Window.GetRequiredExtensionNames();
    extNames.insert(
        extNames.end(), windowExtNames.begin(), windowExtNames.end());

    std::vector<VkExtensionProperties> instanceSupportedExtensions =
        GetInstanceSupportedExtensions();

    if (std::find_if(
            instanceSupportedExtensions.begin(),
            instanceSupportedExtensions.end(),
            [](VkExtensionProperties& properties)
            {
                return properties.extensionName ==
                       VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
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

std::vector<VkExtensionProperties> Video::GetInstanceSupportedExtensions()
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

std::vector<VkPhysicalDevice> Video::GetPhysicalDevices()
{
    VkResult result;
    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        LogError("No vukan devices found!");
    }
    LogVulkanError("Could not fetch physical device count", result);

    std::vector<VkPhysicalDevice> physicalDevices(
        static_cast<size_t>(deviceCount));
    result = vkEnumeratePhysicalDevices(
        m_Instance, &deviceCount, &physicalDevices.front());
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

std::vector<VkDeviceQueueCreateInfo>
Video::GetDeviceQueueCreateInfos(VkPhysicalDevice physicalDevice)
{
    VkResult result;
    uint32_t queueFamilyPropertyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount, nullptr);
    if (queueFamilyPropertyCount == 0)
    {
        LogError("No physical device queues found");
    }
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount,
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
            physicalDevice, i, m_Surface,
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

std::vector<const char*>
Video::GetDeviceExtentionNames(VkPhysicalDevice physicalDevice)
{
    VkResult result;
    uint32_t deviceSupportedExtensionsCount = 0;
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceSupportedExtensionsCount, nullptr);
    LogVulkanError("error getting available device extension count", result);

    std::vector<VkExtensionProperties> deviceSupportedExtensions(
        static_cast<size_t>(deviceSupportedExtensionsCount));
    result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceSupportedExtensionsCount,
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

VkSurfaceCapabilitiesKHR Video::GetSurfaceCapabilities()
{
    VkResult result;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        m_PhysicalDevice, m_Surface, &surfaceCapabilities);

    LogDebug(fmt::format(
        "Surface capabiltiies:\t{}", surfaceCapabilities.currentExtent));
    return surfaceCapabilities;
}

VkSurfaceFormatKHR Video::GetSurfaceFormat()
{
    return PickSurfaceFormat(
        GetCompatableSurfaceFormats(), VK_FORMAT_R8G8B8A8_UNORM);
}

const std::vector<VkSurfaceFormatKHR> Video::GetCompatableSurfaceFormats()
{
    VkResult result;
    uint32_t surfaceFormatCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_PhysicalDevice, m_Surface, &surfaceFormatCount, nullptr);
    LogVulkanError("Failed to get supported surface format count", result);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(
        static_cast<size_t>(surfaceFormatCount));
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_PhysicalDevice, m_Surface, &surfaceFormatCount,
        surfaceFormats.data());
    LogVulkanError(
        "Failed to get surface formats supported by the device", result);

    LogDebug("Supported Formats:");
    for (const auto& surfaceFormat : surfaceFormats)
    {
        LogDebug(fmt::format(
            "\t{}, {}", surfaceFormat.colorSpace, surfaceFormat.format));
    }
    return surfaceFormats;
}

VkSurfaceFormatKHR Video::PickSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& surfaceFormats,
    const VkFormat requestedFormat)
{
    VkSurfaceFormatKHR surfaceFormat = {
        .format = VK_FORMAT_UNDEFINED,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    if (surfaceFormats.size() == 1 &&
        surfaceFormats.at(0).format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR};
    }
    else if (
        std::find_if(
            surfaceFormats.begin(), surfaceFormats.end(),
            [requestedFormat](VkSurfaceFormatKHR surfaceFormat) {
                return surfaceFormat.format == requestedFormat;
            }) != surfaceFormats.end())
    {
        surfaceFormat.format = requestedFormat;
    }

    // fallback
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = surfaceFormats.at(0);
        LogWarning(fmt::format(
            "Requested format not found! Falling back to: {} {}",
            surfaceFormat.format, surfaceFormat.colorSpace));
    }
    return surfaceFormat;
}

std::vector<VkImage> Video::GetSwapchainImages()
{
    VkResult result;
    uint32_t swapchainImageCount = 0;
    result = vkGetSwapchainImagesKHR(
        m_Device, m_Swapchain, &swapchainImageCount, nullptr);
    LogVulkanError("Failed to get swapchain image count", result);
    std::vector<VkImage> swapchainImages(
        static_cast<size_t>(swapchainImageCount));
    result = vkGetSwapchainImagesKHR(
        m_Device, m_Swapchain, &swapchainImageCount, swapchainImages.data());
    LogVulkanError("Failed to get swapchain images", result);
    return swapchainImages;
}
