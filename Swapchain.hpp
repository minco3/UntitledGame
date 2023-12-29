#pragma once
#include "Device.hpp"
#include "Surface.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <vector>

class Swapchain
{
public:
    Swapchain(vk::raii::Device& device, vk::raii::SurfaceKHR& surface,
    vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
    vk::SurfaceFormatKHR& surfaceFormat);
    std::vector<vk::raii::ImageView>& GetImageViews();
    vk::Extent2D GetExtent();
    vk::raii::SwapchainKHR& Get();

private:
    std::vector<vk::raii::ImageView> m_SwapchainImageViews;
    vk::SwapchainCreateInfoKHR GetCreateInfo(
        vk::raii::Device& device, vk::raii::SurfaceKHR& surface,
        vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
        vk::SurfaceFormatKHR& surfaceFormat);
    void CreateSwapchainImageViews(vk::raii::Device& device, vk::SurfaceFormatKHR surfaceFormat);
    vk::raii::SwapchainKHR m_Swapchain;
};