#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>

#ifndef NDEBUG
#define CC_VK_DEBUG_REPORT
#endif

namespace CC
{
    class VulkanMgr
    {
        friend class App;
        friend class ImguiMgr;
    private:
        VulkanMgr() {}
        class VulkanMgrData
        {
        public:
            struct InitRec
            {
            public:
                bool dev;
                bool phyDev;
                bool queueFamily;
                void clear() { dev = phyDev = queueFamily = false; }
            };

            vk::Instance                        instance;
            vk::PhysicalDevice                  physicalDevice;
            vk::Device                          device;
            uint32_t                            queueFamily;
            vk::Queue                           queue;
#ifdef CC_VK_DEBUG_REPORT
            vk::DebugReportCallbackEXT          debugReport;
#endif
            vk::PipelineCache                   pipelineCache;
            vk::DescriptorPool                  descriptorPool;

            uint32_t                            minImageCount = 2;
            InitRec                             initState;

            ~VulkanMgrData() {}
        };
        static std::unique_ptr<VulkanMgrData> _inst;

    public:
        static bool inited();
        static void checkVkResultCtype(VkResult err);
        static void checkVkResult(vk::Result err);

    private:
        static void createInst(const char** extensions, uint32_t extensionsCount);
        static void selectGPU();
        static void selectQueueFamily();
        static void createDevice();
        static void createDescPool();

    private:
        static void init(const char** extensions, uint32_t extensionsCount);
        static void destroy();

    public:
        static vk::Instance& getInst() { return _inst->instance; }
        static vk::PhysicalDevice& getPhyDev() { return _inst->physicalDevice; }
        static vk::Device& getDev() { return _inst->device; }
        static uint32_t getQueueFamily() { return _inst->queueFamily; }
        static vk::Queue& getQueue() { return _inst->queue; }
        static vk::PipelineCache& getPiplineCache() { return _inst->pipelineCache; }
        static vk::DescriptorPool& getDescPool() { return _inst->descriptorPool; }
        static uint32_t getMinImageCount() { return _inst->minImageCount; }
    };
}