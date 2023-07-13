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
    const std::string resultString = string_VkResult(result);
    std::cerr << "ERROR: " << message << ": " << resultString << std::endl;
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

    std::vector<const char*> instanceLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

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
    if (result != VK_SUCCESS || instanceSupportedExtensionCount == 0)
    {
        LogError("Failed to get instance supported extension count", result);
        return 1;
    }
    std::vector<VkExtensionProperties> instanceSupportedExtensions(
        instanceSupportedExtensionCount);
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &instanceSupportedExtensionCount,
        instanceSupportedExtensions.data());
    if (result != VK_SUCCESS)
    {
        LogError("Failed to get instance supported extension count", result);
        return 1;
    }
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
        return 1;
    }

    if (result != VK_SUCCESS)
    {
        LogError("error creating vulkan instance", result);
        return 1;
    }

    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        std::cerr << "ERROR: no vukan devices found!" << std::endl;
        return 1;
    }
    if (result != VK_SUCCESS)
    {
        LogError("error fetching physical device count", result);
        return 1;
    }

    std::vector<VkPhysicalDevice> physicalDevices(
        static_cast<size_t>(deviceCount));
    result = vkEnumeratePhysicalDevices(
        instance, &deviceCount, &physicalDevices.front());
    if (result != VK_SUCCESS)
    {
        LogError("error enumerating physical devices", result);
        return 1;
    }
    else
    {
        std::cout << "Physical Devices:" << std::endl;
        for (uint32_t i = 0; i < deviceCount; i++)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevices.at(i), &properties);
            std::cout << std::setw(4) << ""
                      << "Physical device [" << i
                      << "]: " << properties.deviceName << std::endl;
        }
    }

    VkPhysicalDevice physicalDevice = physicalDevices[0];

    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount, nullptr);
    if (queueFamilyPropertyCount == 0)
    {
        std::cerr << "ERROR: no physical device queues found" << std::endl;
        return 1;
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
        std::cout << std::setw(4) << ""
                  << "Queue Family [" << i
                  << "]: " << std::bitset<32>(properties.queueFlags) << " "
                  << properties.queueCount << " "
                  << properties.timestampValidBits << std::endl;

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
    if (result != VK_SUCCESS)
    {
        LogError("error getting available device extension count", result);
        return 1;
    }
    std::vector<VkExtensionProperties> deviceSupportedExtensions(
        static_cast<size_t>(deviceSupportedExtensionsCount));
    result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceSupportedExtensionsCount,
        deviceSupportedExtensions.data());
    if (result != VK_SUCCESS)
    {
        LogError("error getting available device extensions", result);
        return 1;
    }

#ifdef DEBUG
    std::cout << "Physical Device Supported Extensions:" << std::endl;
    for (const auto properties : deviceSupportedExtensions)
    {
        std::cout << std::setw(4) << " " << properties.extensionName
                  << std::endl;
    }
#endif

    std::vector<const char*> deviceExtensions;

    for (const auto& extension : deviceSupportedExtensions)
    {
        //https://vulkan.lunarg.com/doc/view/1.3.250.1/mac/1.3-extensions/vkspec.html#VUID-VkDeviceCreateInfo-pProperties-04451
        if (!strcmp(extension.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
        {
            deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
        }
    }

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
        return 1;
    }

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