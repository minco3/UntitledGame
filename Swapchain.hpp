#pragma once
#include "Device.hpp"
#include "Surface.hpp"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class Swapchain
{
public:
    Swapchain(Device& device, Surface& surface);

    void Recreate(Device& device, Surface& surface);
    constexpr std::vector<vk::raii::ImageView>& GetImageViews()
    {
        return m_SwapchainImageViews;
    }
    constexpr vk::raii::SwapchainKHR& Get() { return m_Swapchain; }
    constexpr vk::Extent2D GetExtent() const { return m_Extent; }
    constexpr size_t GetImageCount() const { return m_SwapchainImageViews.size(); }

private:
    vk::raii::SwapchainKHR CreateSwapchain(Device& device, Surface& surface);
    std::vector<vk::raii::ImageView> CreateSwapchainImageViews(
        vk::raii::Device& device, vk::SurfaceFormatKHR surfaceFormat);

private:
    vk::raii::SwapchainKHR m_Swapchain;
    std::vector<vk::raii::ImageView> m_SwapchainImageViews;
    vk::Extent2D m_Extent;
};