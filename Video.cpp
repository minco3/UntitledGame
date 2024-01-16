#include "Video.hpp"
#include "Buffer.hpp"
#include "Log.hpp"
#include "Pipeline.hpp"
#include "UniformBuffer.hpp"
#include "Vertex.hpp"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_vulkan.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <algorithm>
#include <fmt/format.h>
#include <glm/common.hpp>
#include <glm/mat2x2.hpp>
#include <span>
#include <vulkan/vulkan_beta.h>

Video::Video()
    : m_Window(
          "Untitled Game", {SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED},
          {1200, 800},
          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN),
      m_Instance(m_Window, m_Context), m_Surface(m_Window, m_Instance),
      m_Device(m_Instance, m_Surface), m_Swapchain(m_Device, m_Surface),
      m_Queue(m_Device.Get(), m_QueueFamilyIndex, 0),
      m_RenderPass(m_Device, m_Surface),
      m_VertexBuffer(
          m_Device, cubeVertices.size(),
          vk::BufferUsageFlagBits::eVertexBuffer),
      m_UniformBuffers(std::move(ConstructUniformBuffers())),
      m_Framebuffers(m_Swapchain, m_RenderPass, m_Device),
      m_CommandBuffers(
          m_Device, m_QueueFamilyIndex, m_Swapchain.GetImageCount()),
      m_SyncObjects(m_Device),
      m_Descriptors(m_Device, m_UniformBuffers, m_Swapchain.GetImageCount()),
      m_Pipeline(m_Device, m_RenderPass, m_Surface, m_Descriptors)
{
    // buffers
    FillVertexBuffer();
}

Video::~Video()
{
    m_Device.Get().waitIdle();
    m_Queue.waitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Video::Render()
{
    vk::raii::Device& device = m_Device.Get();
    device.waitForFences(
        *m_SyncObjects.inFlightFences.at(m_CurrentImage), VK_TRUE,
        std::numeric_limits<uint64_t>::max());

    // Render ImGui frame
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    device.resetFences(*m_SyncObjects.inFlightFences.at(m_CurrentImage));
    auto [result, imageIndex] = m_Swapchain.Get().acquireNextImage(
        std::numeric_limits<uint64_t>::max(),
        *m_SyncObjects.imageAvailableSemaphores.at(m_CurrentImage));

    vk::raii::CommandBuffer& commandBuffer = m_CommandBuffers[m_CurrentImage];

    commandBuffer.reset();

    vk::ClearColorValue clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    vk::ClearValue clearValue(clearColor);

    vk::RenderPassBeginInfo renderPassBeginInfo(
        *m_RenderPass.Get(), *m_Framebuffers[m_CurrentImage],
        vk::Rect2D({}, m_Surface.surfaceCapabilities.currentExtent),
        clearValue);

    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    commandBuffer.beginRenderPass(
        renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, *m_Pipeline.Get());

    vk::DeviceSize offset = 0;

    commandBuffer.bindVertexBuffers(0, *m_VertexBuffer.Get(), offset);

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_Pipeline.GetLayout(), 0,
        *m_Descriptors.GetSets().at(m_CurrentImage), nullptr);

    commandBuffer.draw(static_cast<uint32_t>(cubeVertices.size()), 1, 0, 0);
    ImGui_ImplVulkan_RenderDrawData(draw_data, *commandBuffer);
    commandBuffer.endRenderPass();
    commandBuffer.end();

    vk::PipelineStageFlags waitFlags =
        vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo(
        *m_SyncObjects.imageAvailableSemaphores.at(m_CurrentImage), waitFlags,
        *commandBuffer,
        *m_SyncObjects.renderFinishedSemaphores.at(m_CurrentImage));
    m_Queue.submit(
        submitInfo, *m_SyncObjects.inFlightFences.at(m_CurrentImage));

    vk::PresentInfoKHR presentInfo(
        *m_SyncObjects.renderFinishedSemaphores.at(m_CurrentImage),
        *m_Swapchain.Get(), m_CurrentImage);
    vk::Result presentResult = m_Queue.presentKHR(presentInfo);

    m_CurrentImage = (m_CurrentImage + 1) % m_Swapchain.GetImageCount();
}

void Video::CheckForUpdates()
{
    if (m_Pipeline.NeedsUpdate())
    {
        m_Pipeline.UpdatePipeline(m_Device);
    }
}

void Video::Resize()
{
    m_Surface.GetSurfaceCapabilities(m_Device);
    RecreateRenderables();
    // throw std::runtime_error("debug");
}

void Video::RecreateRenderables()
{
    m_Device.Get().waitIdle();

    m_Swapchain.Clear();
    m_Swapchain = Swapchain(m_Device, m_Surface);
    m_Framebuffers = Framebuffers(m_Swapchain, m_RenderPass, m_Device);
    m_Pipeline = GraphicsPipeline(m_Device, m_RenderPass, m_Surface, m_Descriptors);
}

void Video::RecreatePipeline(const std::string& shaderName, std::filesystem::file_time_type lastModified)
{
    m_Pipeline.Recreate(m_Device, shaderName, lastModified, m_RenderPass, m_Surface);
}

void Video::UpdateUniformBuffers(const glm::mat4& MVP)
{
    m_UniformBuffers.at(m_CurrentImage).GetMemory().front().MVP = MVP;
}

void Video::CaptureCursor(bool state)
{
    SDL_SetWindowMouseGrab(m_Window.Get(), state ? SDL_TRUE : SDL_FALSE);
}

vk::Extent2D Video::GetScreenSize() const { return m_Swapchain.GetExtent(); }

void Video::FillVertexBuffer()
{
    // load hard-coded vertices into memory
    std::span<Vertex> memorySpan = m_VertexBuffer.GetMemory();
    std::copy_n(cubeVertices.begin(), cubeVertices.size(), memorySpan.begin());
}

void Video::InitImGui()
{
    ImGui_ImplSDL2_InitForVulkan(m_Window.Get());
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = *m_Instance.Get();
    init_info.PhysicalDevice = *m_Device.GetPhysicalDevice();
    init_info.Device = *m_Device.Get();
    init_info.QueueFamily = m_QueueFamilyIndex;
    init_info.Queue = *m_Queue;
    init_info.DescriptorPool = *m_Descriptors.GetPool();
    init_info.Subpass = 0;
    init_info.MinImageCount = m_Swapchain.GetImageCount();
    init_info.ImageCount = m_Swapchain.GetImageCount();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    auto check_vulkan_err = [](VkResult err)
    {
        if (vk::Result(err) != vk::Result::eSuccess)
        {
            throw vk::SystemError(std::error_code(vk::Result(err)));
        }
    };
    init_info.CheckVkResultFn = check_vulkan_err;
    ImGui_ImplVulkan_Init(&init_info, *m_RenderPass.Get());
}

std::vector<Buffer<UniformBufferObject>> Video::ConstructUniformBuffers()
{
    std::vector<Buffer<UniformBufferObject>> uniformBuffers;
    for (size_t i = 0; i < m_Swapchain.GetImageCount(); i++)
    {
        uniformBuffers.emplace_back(
            m_Device, 1, vk::BufferUsageFlagBits::eUniformBuffer);
    }

    return uniformBuffers;
}