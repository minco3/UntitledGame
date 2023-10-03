#pragma once

#include "DeviceQueue.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include <vector>
#include <vulkan/vulkan.h>

class Video
{
public:
    Video();
    ~Video();

    void Render();

private:
    void CreateInstance();
    void CreateDevice();
    void CreateSwapchain();
    void CreateSwapchainImageViews();
    void LoadShaders();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandBuffer();
    void CreateVertexBuffer();
    void CreateMemoryBarriers();

    std::vector<VkPipelineShaderStageCreateInfo> CreatePipelineShaderStage();
    std::vector<const char*> GetExtensionNames();
    std::vector<VkExtensionProperties> GetInstanceSupportedExtensions();
    std::vector<VkPhysicalDevice> GetPhysicalDevices();
    std::vector<VkDeviceQueueCreateInfo>
    GetDeviceQueueCreateInfos(VkPhysicalDevice device);
    std::vector<const char*> GetDeviceExtentionNames(VkPhysicalDevice device);
    bool CheckDeviceSupportsPresentation(VkPhysicalDevice device);
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities();
    VkSurfaceFormatKHR GetSurfaceFormat();
    const std::vector<VkSurfaceFormatKHR> GetCompatableSurfaceFormats();
    VkSurfaceFormatKHR PickSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& surfaceFormats,
        const VkFormat requestedFormat);
    std::vector<VkImage> GetSwapchainImages();
    VkPipelineLayout CreatePipelineLayout();
    VkQueue GetQueue(uint32_t queueFamily, uint32_t index);

    std::vector<DeviceQueue> m_DeviceQueues;
    VkInstance m_Instance;
    Window m_Window;
    VkSurfaceKHR m_Surface;
    VkSurfaceFormatKHR m_SurfaceFormat;
    VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSwapchainKHR m_Swapchain;
    std::vector<VkImageView> m_SwapchainImageViews;
    std::vector<VkFramebuffer> m_Framebuffers;
    std::array<Shader, 2> m_ShaderModules = {
        {{ShaderType::Vertex, "shader.vert.spv"},
         {ShaderType::Fragment, "shader.frag.spv"}}};
    VkRenderPass m_RenderPass;
    VkCommandBuffer m_CommandBuffer;
    VkBuffer m_VertexBuffer;
    uint32_t m_QueueFamilyIndex = 0;
    VkQueue m_Queue;
    VkPipeline m_Pipeline;
    VkSemaphore m_Semaphore;
    VkSemaphore m_RenderSemaphore;
    VkFence m_Fence;
};