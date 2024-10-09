#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "Device.hpp"
#include "RenderPass.hpp"
#include "Swapchain.hpp"

class Framebuffers
{
public:
    Framebuffers(Swapchain& swapchain, RenderPass& renderPass, Device& device);
    
    vk::raii::Framebuffer& operator[](size_t index);

private:
    std::vector<vk::raii::Framebuffer> CreateFramebuffers(Swapchain& swapchain, RenderPass& renderPass, Device& device);

    std::vector<vk::raii::Framebuffer> m_Framebuffers;
};