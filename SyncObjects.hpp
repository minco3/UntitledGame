#pragma once

#include "Device.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <cstdint>

constexpr const size_t MAX_FRAMES_IN_FLIGHT=3;

class SyncObjects
{
public:
    SyncObjects(Device& device);

    std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence>     inFlightFences;

};