#pragma once
#include "Device.hpp"
#include "Surface.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class Swapchain
{
public:
    Swapchain(Device& device, Surface& surface);

    constexpr std::vector<vk::raii::ImageView>& GetImageViews()
    {
        return m_SwapchainImageViews;
    }
    constexpr vk::raii::SwapchainKHR& Get() { return m_Swapchain; }
    constexpr vk::Extent2D GetExtent() { return m_Extent; }
    constexpr size_t GetImageCount() { return m_ImageCount; }

private:
    std::vector<vk::raii::ImageView> m_SwapchainImageViews;
    vk::raii::SwapchainKHR CreateSwapchain(
        Device& device, Surface& surface);
    void CreateSwapchainImageViews(
        vk::raii::Device& device, vk::SurfaceFormatKHR surfaceFormat);
    vk::raii::SwapchainKHR m_Swapchain;
    size_t m_ImageCount;
    vk::Extent2D m_Extent;
};