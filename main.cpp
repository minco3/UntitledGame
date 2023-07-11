#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#include <vma/vk_mem_alloc.h>

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
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
    createInfo.ppEnabledExtensionNames = extNames.data();

    VkAllocationCallbacks allocator;
    VkInstance instance;

    const VkResult result =
        vkCreateInstance(&createInfo, &allocator, &instance);
        
    if (result != VK_SUCCESS)
    {
        std::string resultString = string_VkResult(result);
        std::cout << "Error creating vulkan instance: " << resultString
                  << std::endl;
        return 1;
    }

    VkSurfaceKHR surface;
    VkDisplaySurfaceCreateInfoKHR dsCreateInfo;
    vkCreateDisplayPlaneSurfaceKHR(
        instance, &dsCreateInfo, &allocator, &surface
    );
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