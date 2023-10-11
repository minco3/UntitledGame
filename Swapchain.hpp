#pragma once
#include "Device.hpp"
#include "Surface.hpp"
#include "vulkan/vulkan_core.h"
#include <vector>

class Swapchain
{
public:
    Swapchain(Device& device, Surface& surface);
    VkSwapchainKHR& operator()();
    std::vector<VkImageView> m_SwapchainImageViews;
private:
    void CreateSwapchain(Device& device, Surface& surface);
    void CreateSwapchainImageViews(Device& device, Surface& surface);
    std::vector<VkImage> GetSwapchainImages(Device& device);
    VkSwapchainKHR m_Swapchain;
};