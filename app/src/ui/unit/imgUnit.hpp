#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "entrance.h"

namespace CC::UI
{
    static inline uint32_t findMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties mem_prop = VulkanMgr::getPhyDev().getMemoryProperties();

        for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++)
            if ((type_filter & (1 << i)) && (mem_prop.memoryTypes[i].propertyFlags & properties) == properties)
                return i;

        return 0xFFFFFFFF;
    }

    /// @brief 图像组件
    class ImageUnit
    {
    private:
        VkDescriptorSet _ds;
        int _w;
        int _h;
        int _channels;

        vk::ImageView _imgView;
        vk::Image _img;
        vk::DeviceMemory _imgMemory;
        vk::Sampler _sampler;
        vk::Buffer _uBuf;
        vk::DeviceMemory _uBufMemory;

        bool _isReleased = false;

    public:
        ImageUnit(uint8_t* buf, size_t size, int w, int h, int channels,
        std::function<void(uint8_t*, size_t)> releaseHandle = stbRelease) :
        _w(w), _h(h), _channels(channels)
        {
            if (_w <= 0 || _h <= 0) { _isReleased = true; return; }
            loadAndReleaseData(buf, size, releaseHandle);
        }

        ImageUnit(const char* filename)
        {
            // --------------------- 加载图像文件到内存
            _channels = 4;
            uint8_t* data = stbi_load(filename, &_w, &_h, 0, _channels);
            if (!data) throw Exce(__LINE__, __FILE__, "ImgUnit: 读取图像文件错误");
            size_t size = _w * _h * _channels;

            // --------------------- 提呈到 vk
            loadAndReleaseData(data, size);
        }

        void release()
        {
            if (_isReleased) return;
            auto& dev = VulkanMgr::getDev();
            ImGui_ImplVulkan_RemoveTexture(_ds);
            dev.freeMemory(_uBufMemory);
            dev.destroyBuffer(_uBuf);
            dev.destroySampler(_sampler);
            dev.destroyImageView(_imgView);
            dev.destroyImage(_img);
            dev.freeMemory(_imgMemory);
            _isReleased = true;
        }

        ImTextureID getId()
        {
            return _ds;
        }

        virtual ~ImageUnit() { release(); }

    private:
        static void stbRelease(uint8_t* data, size_t size)
        {
            stbi_image_free(data);
        }

        void loadAndReleaseData(uint8_t* data, size_t size, std::function<void(uint8_t*, size_t)> releaseHandle = stbRelease)
        {
            // 申请内存和显存
            vk::ImageCreateInfo info;
            info.imageType = vk::ImageType::e2D;
            info.format = vk::Format::eR8G8B8A8Unorm;
            info.extent.width = _w;
            info.extent.height = _h;
            info.extent.depth = 1;
            info.arrayLayers = 1;
            info.mipLevels = 1;
            info.samples = vk::SampleCountFlagBits::e1;
            info.tiling = vk::ImageTiling::eOptimal;
            info.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
            info.sharingMode = vk::SharingMode::eExclusive;
            info.initialLayout = vk::ImageLayout::eUndefined;

            _img = VulkanMgr::getDev().createImage(info);
            vk::MemoryRequirements req = VulkanMgr::getDev().getImageMemoryRequirements(_img);
            vk::MemoryAllocateInfo alloc_info;
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = findMemoryType(req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
            _imgMemory = VulkanMgr::getDev().allocateMemory(alloc_info);
            VulkanMgr::getDev().bindImageMemory(_img, _imgMemory, 0);

            // 创建视图和采样器
            vk::ImageViewCreateInfo view_info;
            view_info.image = _img;
            view_info.viewType = vk::ImageViewType::e2D;
            view_info.format = vk::Format::eR8G8B8A8Unorm;
            view_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.layerCount = 1;
            _imgView = VulkanMgr::getDev().createImageView(view_info);

            vk::SamplerCreateInfo sampler_info;
            sampler_info.magFilter = vk::Filter::eNearest;
            sampler_info.minFilter = vk::Filter::eNearest;
            sampler_info.mipmapMode = vk::SamplerMipmapMode::eNearest;
            sampler_info.addressModeU = vk::SamplerAddressMode::eRepeat;
            sampler_info.addressModeV = vk::SamplerAddressMode::eRepeat;
            sampler_info.addressModeW = vk::SamplerAddressMode::eRepeat;
            sampler_info.minLod = -1000;
            sampler_info.maxLod = 1000;
            sampler_info.maxAnisotropy = 1.0f;
            _sampler = VulkanMgr::getDev().createSampler(sampler_info);

            // 引入 imgui 接口
            _ds = ImGui_ImplVulkan_AddTexture(_sampler, _imgView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            // 创建内存映射并绑定到显存中 并在提呈后解绑
            vk::BufferCreateInfo buf_info;
            buf_info.size = size;
            buf_info.usage = vk::BufferUsageFlagBits::eTransferSrc;
            buf_info.sharingMode = vk::SharingMode::eExclusive;
            _uBuf = VulkanMgr::getDev().createBuffer(buf_info);
            req = VulkanMgr::getDev().getBufferMemoryRequirements(_uBuf);
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = findMemoryType(req.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
            _uBufMemory = VulkanMgr::getDev().allocateMemory(alloc_info);
            VulkanMgr::getDev().bindBufferMemory(_uBuf, _uBufMemory, 0);

            // 提呈到显存
            void* map = VulkanMgr::getDev().mapMemory(_uBufMemory, 0, size);
            memcpy(map, data, size);
            std::array<vk::MappedMemoryRange, 1> ranges = {};
            ranges[0].setMemory(_uBufMemory);
            ranges[0].size = size;
            VulkanMgr::getDev().flushMappedMemoryRanges(ranges);
            VulkanMgr::getDev().unmapMemory(_uBufMemory);

            // 释放内存
            releaseHandle(data, size);

            // 提呈渲染指令
            vk::CommandPool cmdPool = ImguiMgr::getHWnd().Frames[ImguiMgr::getHWnd().FrameIndex].CommandPool;
            std::vector<vk::CommandBuffer> cmdBuf;
            vk::CommandBufferAllocateInfo cmdAlloc_info;
            cmdAlloc_info.level = vk::CommandBufferLevel::ePrimary;
            cmdAlloc_info.commandPool = cmdPool;
            cmdAlloc_info.commandBufferCount = 1;
            cmdBuf = VulkanMgr::getDev().allocateCommandBuffers(cmdAlloc_info);

            vk::CommandBufferBeginInfo begin_info;
            begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
            cmdBuf.back().begin(begin_info);

            // 复制数据
            vk::ImageMemoryBarrier cBarrier;
            cBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            cBarrier.oldLayout = vk::ImageLayout::eUndefined;
            cBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
            cBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            cBarrier.image = _img;
            cBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            cBarrier.subresourceRange.levelCount = 1;
            cBarrier.subresourceRange.layerCount = 1;
            cmdBuf.back().pipelineBarrier(
                vk::PipelineStageFlagBits::eHost,
                vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, {cBarrier});

            vk::BufferImageCopy region;
            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.layerCount = 1;
            region.imageExtent.width = _w;
            region.imageExtent.height = _h;
            region.imageExtent.depth = 1;
            cmdBuf.back().copyBufferToImage(_uBuf, _img, vk::ImageLayout::eTransferDstOptimal, {region});

            vk::ImageMemoryBarrier uBarrier;
            uBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            uBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            uBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            uBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            uBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            uBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            uBarrier.image = _img;
            uBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            uBarrier.subresourceRange.levelCount = 1;
            uBarrier.subresourceRange.layerCount = 1;
            cmdBuf.back().pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, {uBarrier});

            vk::SubmitInfo end_info;
            end_info.setCommandBuffers(cmdBuf);
            cmdBuf.back().end();
            VulkanMgr::getQueue().submit(end_info);
            VulkanMgr::getDev().waitIdle();
        }
    };
}