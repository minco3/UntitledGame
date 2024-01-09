#include "SyncObjects.hpp"

SyncObjects::SyncObjects(Device& device)
{

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        imageAvailableSemaphores.emplace_back(
            device.Get(), vk::SemaphoreCreateInfo());
        renderFinishedSemaphores.emplace_back(
            device.Get(), vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(
            device.Get(),
            vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }
}