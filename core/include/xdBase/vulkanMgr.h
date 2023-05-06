#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

#ifndef NDEBUG
#define CC_VK_DEBUG_REPORT
#endif

namespace XD::VulkanMgr {
        bool inited();
        void checkVkResultCType(VkResult err);
        void checkVkResult(vk::Result err);

        vk::Instance& getInst();
        vk::PhysicalDevice& getPhyDev();
        vk::Device& getDev();
        uint32_t getQueueFamily();
        vk::Queue& getQueue();
        vk::PipelineCache& getPipelineCache();
        vk::DescriptorPool& getDescPool();
        uint32_t getMinImageCount();
}