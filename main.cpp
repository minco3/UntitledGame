#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#define VMA_IMPLEMENTATION
#include <bitset>
#include <vma/vk_mem_alloc.h>

void LogError(const std::string& message, VkResult result)
{
    const std::string resultString = string_VkResult(result);
    std::cerr << message << resultString << std::endl;
}

int main()
{
    bool running = true;
    SDL_Window* window;
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "Marcocraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
    );

    std::vector<const char*> extNames = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME};

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
    createInfo.ppEnabledExtensionNames = extNames.data();

    VkInstance instance;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS)
    {
        LogError("ERROR creating vulkan instance: ", result);
        return 1;
    }

    uint32_t deviceCount;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LogError("ERROR fetching physical device count: ", result);
        return 1;
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(deviceCount);
    result = vkEnumeratePhysicalDevices(
        instance, &deviceCount, &physicalDevices.front()
    );
    if (result != VK_SUCCESS)
    {
        LogError("ERROR enumerating physical devices: ", result);
        return 1;
    }
    else
    {
        std::cout << "Physical Devices:" << std::endl;
        for (uint32_t i = 0; i < deviceCount; i++)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevices.at(i), &properties);
            std::cout << std::setw(4) << "" << "Physical device [" << i
                      << "]: " << properties.deviceName << std::endl;
        }
    }

    VkPhysicalDevice physicalDevice = physicalDevices[0];

    uint32_t queueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount, nullptr
    );
    if (queueFamilyPropertyCount == 0)
    {
        std::cerr << "ERROR No physical device queues" << std::endl;
        return 1;
    }
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    queueFamilyProperties.resize(queueFamilyPropertyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyPropertyCount,
        &queueFamilyProperties.front()
    );

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

    std::cout << "Queues:" << std::endl;
    for (uint32_t i = 0; i < queueFamilyPropertyCount; i++)
    {
        const VkQueueFamilyProperties& properties = queueFamilyProperties.at(i);
        std::cout << std::setw(4) << "" << "Queue [" << i
                  << "]: " << std::bitset<32>(properties.queueFlags) << " "
                  << properties.queueCount << " "
                  << properties.timestampValidBits << std::endl;

        VkDeviceQueueCreateInfo deviceQueueCreateInfo;
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueFamilyIndex = i;
        deviceQueueCreateInfo.queueCount = properties.queueCount;
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    }

    VkDevice device = {};
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfos.front();
    deviceCreateInfo.queueCreateInfoCount = 1;

    result =
        vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        LogError("Error creating device: ", result);
        return 1;
    }
    
    VkSurfaceKHR surface;
    VkDisplaySurfaceCreateInfoKHR dsCreateInfo;
    vkCreateDisplayPlaneSurfaceKHR(instance, &dsCreateInfo, nullptr, &surface);
    SDL_Vulkan_CreateSurface(window, instance, &surface);

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