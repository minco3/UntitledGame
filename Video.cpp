#include "Video.hpp"
#include "Buffer.hpp"
#include "Log.hpp"
#include "Pipeline.hpp"
#include "UniformBuffer.hpp"
#include "Vertex.hpp"
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
          {1200, 800}, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN),
      m_Instance(m_Window, m_Context), m_Surface(m_Window, m_Instance),
      m_Device(m_Instance, m_Surface), m_Swapchain(m_Device, m_Surface),
      m_Queue(m_Device.Get(), m_QueueFamilyIndex, 0),
      m_RenderPass(m_Device, m_Surface),
      m_VertexBuffer(
          m_Device, vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer),
      m_UniformBuffers(std::move(ConstructUniformBuffers())),
      m_Descriptors(m_Device, m_UniformBuffers, m_Swapchain.GetImageCount()),
      m_Pipeline(m_Device, m_RenderPass, m_Surface, m_Descriptors),
      m_Framebuffers(m_Swapchain, m_RenderPass, m_Device),
      m_CommandBuffer(m_Device, m_QueueFamilyIndex), m_SyncObjects(m_Device)
{
    // buffers
    FillVertexBuffer();
}
Video::~Video() {}

void Video::Render()
{
    vk::raii::Device& device = m_Device.Get();
    device.waitForFences(
        *m_SyncObjects.inFlightFences.at(m_CurrentImage), true,
        std::numeric_limits<uint64_t>::max());
    device.resetFences(*m_SyncObjects.inFlightFences.at(m_CurrentImage));
    vk::AcquireNextImageInfoKHR nextImageInfo(
        *m_Swapchain.Get(), std::numeric_limits<uint64_t>::max(),
        *m_SyncObjects.imageAvailableSemaphores.at(m_CurrentImage));
    auto [result, m_CurrentImage] = device.acquireNextImage2KHR(nextImageInfo);

    m_CommandBuffer.Get().reset();

    vk::ClearColorValue clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    vk::ClearValue clearValue(clearColor);

    vk::RenderPassBeginInfo renderPassBeginInfo(
        *m_RenderPass.Get(), *m_Framebuffers[m_CurrentImage],
        vk::Rect2D({}, m_Surface.surfaceCapabilities.currentExtent),
        clearValue);

    vk::raii::CommandBuffer& commandBuffer = m_CommandBuffer.Get();

    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    commandBuffer.beginRenderPass(
        renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, *m_Pipeline.Get());

    vk::DeviceSize deviceSize = 0;

    commandBuffer.bindVertexBuffers(0, *m_VertexBuffer.Get(), deviceSize);

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, *m_Pipeline.GetLayout(), 0,
        *m_Descriptors.GetSets().at(m_CurrentImage), nullptr);

    commandBuffer.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();

    vk::PipelineStageFlags waitFlags =
        vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo(
        *m_SyncObjects.imageAvailableSemaphores.at(m_CurrentImage), waitFlags,
        *m_CommandBuffer.Get(),
        *m_SyncObjects.renderFinishedSemaphores.at(m_CurrentImage));
    m_Queue.submit(
        submitInfo, *m_SyncObjects.inFlightFences.at(m_CurrentImage));

    vk::PresentInfoKHR presentInfo =
        (m_SyncObjects.renderFinishedSemaphores.at(m_CurrentImage),
         *m_Swapchain.Get(), m_CurrentImage);
    vk::Result presentResult = m_Queue.presentKHR(presentInfo);
}

void Video::UpdateUnformBuffers(float theta)
{
    UniformBufferObject& buffer =
        m_UniformBuffers.at(m_CurrentImage).GetMemory().front();
    buffer.rotation[0].x = cos(theta * M_PI / 180);
    buffer.rotation[0].y = -sin(theta * M_PI / 180);
    buffer.rotation[1].x = sin(theta * M_PI / 180);
    buffer.rotation[1].y = cos(theta * M_PI / 180);
    buffer.colorRotation = theta;
}

void Video::FillVertexBuffer()
{
    // load hard-coded vertices into memory
    std::span<Vertex> memorySpan = m_VertexBuffer.GetMemory();
    std::copy_n(vertices.begin(), vertices.size(), memorySpan.begin());
}

std::vector<Buffer<UniformBufferObject>> Video::ConstructUniformBuffers()
{
    std::vector<Buffer<UniformBufferObject>> uniformBuffers;
    for (size_t i = 0; i < m_Swapchain.GetImageCount(); i++)
    {
        uniformBuffers.emplace_back(m_Device, 1, vk::BufferUsageFlagBits::eUniformBuffer);
    }

    return uniformBuffers;
}