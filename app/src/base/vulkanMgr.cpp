#include "vulkanMgr.h"

#include <cassert>
#ifdef CC_VK_DEBUG_REPORT
#include <cstdio>         // printf, fprintf
#include <cstdlib>        // abort
#endif
#include <cstring>        // memcpy

#include "ccexce.h"

namespace CC
{
#ifdef CC_VK_DEBUG_REPORT
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugReport(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t object, size_t location,
        int32_t messageCode,
        const char* pLayerPrefix,
        const char* pMessage,
        void* pUserData)
    {
        (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
        fprintf(stderr, "CC::Vulkan 来自对象类型为 %i 的 Debug 报告: \n\nMessage: %s\n\n", objectType, pMessage);
        return VK_FALSE;
    }
#endif

    std::unique_ptr<VulkanMgr::VulkanMgrData> VulkanMgr::_inst = nullptr;

    bool VulkanMgr::inited() { return (bool)_inst; }

    void VulkanMgr::checkVkResultCtype(VkResult err)
    {
        if (err == 0) return;
        throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: Result = " + (int)err);
        if (err < 0) abort();
    }

    void VulkanMgr::checkVkResult(vk::Result err)
    {
        if (err == vk::Result::eSuccess) return;
        throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: Result = " + (int)err);
    }

    void VulkanMgr::createInst(const char** extensions, uint32_t extensionsCount)
    {
        if (_inst) return;
        _inst = std::make_unique<VulkanMgrData>();

        // 创建 vk 实例并应用校验层
        vk::InstanceCreateInfo insInfo;
        insInfo.setEnabledExtensionCount(extensionsCount);
        insInfo.setPpEnabledExtensionNames(extensions);

#ifdef CC_VK_DEBUG_REPORT
        // 校验层
        const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
        insInfo.setEnabledLayerCount(1);
        insInfo.setPpEnabledLayerNames(layers);

        // 扩展报告
        uint32_t extensionsExtCount = 1;
        const char** extensionsExt = (const char**)malloc(sizeof(const char*) * (extensionsCount + extensionsExtCount));
        memcpy(extensionsExt, extensions, extensionsCount * sizeof(const char*));
        extensionsExt[extensionsCount] = "VK_EXT_debug_report";
        insInfo.setEnabledExtensionCount(extensionsCount + extensionsExtCount);
        insInfo.setPpEnabledExtensionNames(extensionsExt);

        // 创建 vk 实例
        _inst->instance = vk::createInstance(insInfo);
        free(extensionsExt);

        // 创建校验报告回调函数
        auto vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkDebugReportCallbackEXT>(debugReport);
        auto debugMessengerInfo = vk::DebugReportCallbackCreateInfoEXT()
            .setFlags(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning)
            .setPfnCallback(vkCreateDebugReportCallbackEXT)
            .setPUserData(nullptr);

        vk::DynamicLoader dl;
        auto GetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        vk::DispatchLoaderDynamic dispatch((_inst->instance), GetInstanceProcAddr);
        if (_inst->instance.createDebugReportCallbackEXT(&debugMessengerInfo, 0, &(_inst->debugReport), dispatch) != vk::Result::eSuccess)
        { throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: 初始化调试信息失败"); }
#else
        // 创建 vk 实例
        _inst->instance = vk::createInstance(insInfo);
#endif
    }

    void VulkanMgr::selectGPU()
    {
        if (!_inst) throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: Instance Empty");
        if (_inst->initState.phyDev) throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: 重复选择 GPU");
        auto allGPU = _inst->instance.enumeratePhysicalDevices();
        if (allGPU.empty()) throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: 找不到可用 GPU");

        uint32_t selectedGpu = 0;
        for (auto i = std::size_t(0); i < selectedGpu; i++)
        {
            if (allGPU[i].getProperties().deviceType ==
                vk::PhysicalDeviceType::eDiscreteGpu)
            {
                selectedGpu = i;
                break;
            }
        }
        _inst->physicalDevice = allGPU[selectedGpu];
        _inst->initState.phyDev = true;
    }

    void VulkanMgr::createDevice()
    {
        if (!_inst) throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: Instance Empty");
        if (!(_inst->initState.queueFamily)) throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: 没有初始化队列簇");

        int devExtCount = 1;
        const char* devExt[] = { "VK_KHR_swapchain" };
        const float queuePriority[] = { 1.0f };
        vk::DeviceQueueCreateInfo queueInfo[1] = {};
        queueInfo[0].setQueueFamilyIndex(_inst->queueFamily);
        queueInfo[0].setQueueCount(1);
        queueInfo[0].setPQueuePriorities(queuePriority);
        vk::DeviceCreateInfo devInfo;
        devInfo.setQueueCreateInfoCount(sizeof(queueInfo) / sizeof(queueInfo[0]));
        devInfo.setPQueueCreateInfos(queueInfo);
        devInfo.setEnabledExtensionCount(devExtCount);
        devInfo.setPpEnabledExtensionNames(devExt);
        _inst->device = _inst->physicalDevice.createDevice(devInfo);
        _inst->queue = _inst->device.getQueue(_inst->queueFamily, 0);
        _inst->initState.dev = true;
    }

    void VulkanMgr::selectQueueFamily()
    {
        if (!_inst) throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: Instance Empty");
        if (!(_inst->initState.phyDev)) throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: 没有初始化 GPU");
        auto found = false;
        auto _queueProp = _inst->physicalDevice.getQueueFamilyProperties();
        for(auto i = 0; i < _queueProp.size(); i++)
        {
            if (_queueProp[i].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                _inst->queueFamily = i;
                found = true;
                _inst->initState.queueFamily = true;
                return;
            }
        }
    }

    void VulkanMgr::createDescPool()
    {
        if (!_inst) throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: Instance Empty");
        if (!(_inst->initState.dev)) throw Exce(__LINE__, __FILE__, "CC::VulkanMgr Exce: 没有初始化 Dev");

#define ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*(_ARR))))
        vk::DescriptorPoolSize poolSizes[] =
        {
            {vk::DescriptorType::eSampler, 1000},
            {vk::DescriptorType::eCombinedImageSampler, 1000},
            {vk::DescriptorType::eSampledImage, 1000},
            {vk::DescriptorType::eStorageImage, 1000},
            {vk::DescriptorType::eUniformTexelBuffer, 1000},
            {vk::DescriptorType::eStorageTexelBuffer, 1000},
            {vk::DescriptorType::eUniformBuffer, 1000},
            {vk::DescriptorType::eStorageBuffer, 1000},
            {vk::DescriptorType::eUniformBufferDynamic, 1000},
            {vk::DescriptorType::eStorageBufferDynamic, 1000},
            {vk::DescriptorType::eInputAttachment, 1000}
        };
        uint32_t poolSizesSize = ARRAYSIZE(poolSizes);
        vk::DescriptorPoolCreateInfo poolInfo = {};
        poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        poolInfo.setMaxSets(1000 * poolSizesSize);
        poolInfo.setPoolSizeCount(poolSizesSize);
        poolInfo.setPPoolSizes(poolSizes);
        auto err = _inst->device.createDescriptorPool(&poolInfo, nullptr, &(_inst->descriptorPool));
        checkVkResult(err);
#undef ARRAYSIZE
    }

    void VulkanMgr::init(const char** extensions, uint32_t extensionsCount)
    {
        if (_inst) return;

        createInst(extensions, extensionsCount);
        selectGPU();
        selectQueueFamily();
        createDevice();
        createDescPool();
    }

    void VulkanMgr::destroy()
    {
        vkDestroyDescriptorPool(_inst->device, _inst->descriptorPool, nullptr);
#ifdef CC_VK_DEBUG_REPORT
        auto destroyFunc = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr((_inst->instance), "vkDestroyDebugReportCallbackEXT");
        destroyFunc((_inst->instance), (_inst->debugReport), nullptr);
#endif
        vkDestroyDevice(_inst->device, nullptr);
        vkDestroyInstance(_inst->instance, nullptr);
        _inst.reset();
    }
}
