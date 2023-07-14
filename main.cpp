#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_vulkan.h>
#include <iomanip>
#include <iostream>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#define VMA_IMPLEMENTATION
#include <bitset>
#include <vma/vk_mem_alloc.h>

// #define DEBUG

void LogError(const std::string& message, VkResult result)
{
    if (result != VK_SUCCESS)
    {
        const std::string resultString = string_VkResult(result);
        std::cerr << "ERROR: " << message << ": " << resultString << std::endl;
        exit(1);
    }
}

int main()
{

    VkResult result;
    bool running = true;
    SDL_Window* window;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "ERROR: error initializing sdl: " << SDL_GetError()
                  << std::endl;
    }

    window = SDL_CreateWindow(
        "Marcocraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (window == nullptr)
    {
        std::cerr << "ERROR: error creating SDL window: " << SDL_GetError()
                  << std::endl;
    }

    std::vector<const char*> extNames = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME};

    std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    unsigned int numExtentions;
    if (SDL_Vulkan_GetInstanceExtensions(window, &numExtentions, nullptr) !=
        SDL_TRUE)
    {
        std::cerr << "ERROR: error getting SDL required vulkan exension count"
                  << std::endl;
    }
    std::vector<const char*> sdlExtNames(numExtentions);
    if (SDL_Vulkan_GetInstanceExtensions(
            window, &numExtentions, sdlExtNames.data()) != SDL_TRUE)
    {
        std::cerr << "ERROR: error getting SDL required vulkan extensions"
                  << std::endl;
    }

    extNames.insert(extNames.end(), sdlExtNames.begin(), sdlExtNames.end());

    uint32_t instanceSupportedExtensionCount;
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &instanceSupportedExtensionCount, nullptr);
    LogError("Failed to get instance supported extension count", result);

    std::vector<VkExtensionProperties> instanceSupportedExtensions(
        instanceSupportedExtensionCount);
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &instanceSupportedExtensionCount,
        instanceSupportedExtensions.data());
    LogError("Failed to get instance supported extension count", result);

#ifdef DEBUG
    std::cout << "Instance Supported Extensions:" << std::endl;
    for (const VkExtensionProperties& properties : instanceSupportedExtensions)
    {
        std::cout << std::setw(4) << " " << properties.extensionName
                  << std::endl;
    }
#endif
    for (const char* extName : extNames)
    {
        for (uint32_t i = 0; i < instanceSupportedExtensionCount; i++)
        {
            const VkExtensionProperties& properties =
                instanceSupportedExtensions.at(i);
            if (!strcmp(extName, properties.extensionName))
            {
                break;
            }
            else if (i == instanceSupportedExtensionCount - 1)
            {
                std::cerr << "ERROR: Requested extension not supported!"
                          << std::endl;
                return 1;
            }
        }
    }

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_3,
        .applicationVersion = 1,
        .pApplicationName = "SDL TEST",
        .pEngineName = nullptr};

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .enabledExtensionCount = static_cast<uint32_t>(extNames.size()),
        .ppEnabledExtensionNames = extNames.data(),
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data()};

    VkInstance instance;

    result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE)
    {
        std::cerr << "ERROR: error creating surface" << std::endl;
        exit(1);
    }

    LogError("error creating vulkan instance", result);

    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        std::cerr << "ERROR: no vukan devices found!" << std::endl;
        exit(1);
    }
    LogError("error fetching physical device count", result);

    std::vector<VkPhysicalDevice> physicalDevices(
        static_cast<size_t>(deviceCount));
    result = vkEnumeratePhysicalDevices(
        instance, &deviceCount, &physicalDevices.front());
    LogError("error enumerating physical devices", result);
    
    std::cout << "Physical Devices:" << std::endl;
    for (uint32_t i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices.at(i), &properties);
        std::cout << std::setw(4) << ""
                    << "Physical device [" << i
                    << "]: " << properties.deviceName << std::endl;
    }

    VkPhysicalDevice physicalDevice = physicalDevices[0];

    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount, nullptr);
    if (queueFamilyPropertyCount == 0)
    {
        std::cerr << "ERROR: no physical device queues found" << std::endl;
        exit(1);
    }
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount,
        queueFamilyProperties.data());

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos(
        static_cast<size_t>(queueFamilyPropertyCount));
    std::vector<const float> deviceQueuePriorities(
        static_cast<size_t>(queueFamilyPropertyCount), 1.0f);

    std::cout << "Queues:" << std::endl;
    for (uint32_t i = 0; i < queueFamilyPropertyCount; i++)
    {
        const VkQueueFamilyProperties& properties = queueFamilyProperties.at(i);

        VkBool32 supported;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice, i, surface, &supported);
        LogError("failed to get physical device support", result);

        std::cout << std::setw(4) << ""
                  << "Queue Family [" << i
                  << "]: " << std::bitset<32>(properties.queueFlags) << " "
                  << properties.queueCount << " " << std::boolalpha << supported
                  << " " << properties.timestampValidBits << std::endl;

        VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = i,
            .queueCount = properties.queueCount,
            .pQueuePriorities = &deviceQueuePriorities.at(i)};
        deviceQueueCreateInfos.at(i) = (deviceQueueCreateInfo);
    }

    uint32_t deviceSupportedExtensionsCount = 0;
    result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceSupportedExtensionsCount, nullptr);
    LogError("error getting available device extension count", result);

    std::vector<VkExtensionProperties> deviceSupportedExtensions(
        static_cast<size_t>(deviceSupportedExtensionsCount));
    result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceSupportedExtensionsCount,
        deviceSupportedExtensions.data());
    LogError("error getting available device extensions", result);

#ifdef DEBUG
    std::cout << "Physical Device Supported Extensions:" << std::endl;
    for (const auto properties : deviceSupportedExtensions)
    {
        std::cout << std::setw(4) << " " << properties.extensionName
                  << std::endl;
    }
#endif

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

    uint32_t surfaceFormatCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice, surface, &surfaceFormatCount, nullptr);
        LogError("Failed to get physical device format count", result);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(
        static_cast<size_t>(surfaceFormatCount));
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());
        LogError("Failed to get physical device formats", result);

#ifdef DEBUG
    std::cout << "Supported Formats:" << std::endl;
    for (const auto& surfaceFormat : surfaceFormats)
    {
        std::cout << std::setw(4) << " "
                  << string_VkColorSpaceKHR(surfaceFormat.colorSpace) << " "
                  << string_VkFormat(surfaceFormat.format) << std::endl;
    }
#endif

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice, surface, &surfaceCapabilities);

    std::cout << "Surface capabiltiies:" << std::endl;
    std::cout << std::setw(4) << " "
              << "Current Extent: " << surfaceCapabilities.currentExtent.width
              << "x" << surfaceCapabilities.currentExtent.height << std::endl;

    VkDevice device = {};
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount =
            static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        .pQueueCreateInfos = deviceQueueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data()};

    result =
        vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        LogError("error creating device", result);
        exit(1);
    }

    uint32_t numImages = surfaceCapabilities.minImageCount + 1;

    VkSurfaceFormatKHR surfaceFormat = {.format = VK_FORMAT_UNDEFINED};
    if (surfaceFormats.size() == 1 &&
        surfaceFormats.at(0).format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR};
    }
    else
    {
        for (const VkSurfaceFormatKHR& format : surfaceFormats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                surfaceFormat = format;
                break;
            }
        }
    }
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = surfaceFormats.at(0);
        std::cerr << "WARNING: Requested format not found! falling back to: "
                  << string_VkFormat(surfaceFormat.format) << " "
                  << string_VkColorSpaceKHR(surfaceFormat.colorSpace)
                  << std::endl;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = numImages,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .imageArrayLayers = 1,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = true,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .imageExtent = surfaceCapabilities.currentExtent};

    VkSwapchainKHR swapchain;
    result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
    LogError("failed to create swapchain", result);

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    running = false;
                    break;
                }
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}