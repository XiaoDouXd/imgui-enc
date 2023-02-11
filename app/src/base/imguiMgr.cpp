#include "ccexce.h"
#include "imgui_impl_sdl.h"
#include "imguiMgr.h"
#include "vulkanMgr.h"

#include "FONTS_SOURCEHANSANSCN_NORMAL_OTF.h"
#include "FONTS_______________________UCC8105___1_0X00_.h"

namespace CC
{
    std::unique_ptr<ImguiMgr::ImguiMgrData> ImguiMgr::_inst = nullptr;

    bool ImguiMgr::inited() { return (bool)_inst; }

    bool ImguiMgr::notLoadingFinished() { return _inst->notloadingRendered; }

    void ImguiMgr::init(SDL_Window* wSdl, VkSurfaceKHR& surf, int w, int h)
    {
        if (_inst) return;
        if (!VulkanMgr::inited()) throw Exce(__LINE__, __FILE__, "CC::ImguiMgr: 未初始化 vulkan");
        _inst = std::make_unique<ImguiMgrData>();

        // 初始化 imgui 上下文
        IMGUI_CHECKVERSION();
        _inst->context = ImGui::CreateContext();
        ImGui::SetCurrentContext(_inst->context);
        _inst->io = &(ImGui::GetIO());

        _inst->wHandle.Surface = surf;
        auto res = VulkanMgr::getPhyDev().getSurfaceSupportKHR(VulkanMgr::getQueueFamily(), surf);
        if (res != VK_TRUE) throw Exce(__LINE__, __FILE__, "CC::App: 不支持的物理设备");

        const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        _inst->wHandle.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(VulkanMgr::getPhyDev(), _inst->wHandle.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

#ifdef IMGUI_UNLIMITED_FRAME_RATE
        VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
        VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
        _inst->wHandle.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(VulkanMgr::getPhyDev(), _inst->wHandle.Surface, &presentModes[0], IM_ARRAYSIZE(presentModes));

        IM_ASSERT(VulkanMgr::getMinImageCount() >= 2);
        ImGui_ImplVulkanH_CreateOrResizeWindow(
            VulkanMgr::getInst(),
            VulkanMgr::getPhyDev(),
            VulkanMgr::getDev(),
            &(_inst->wHandle),
            VulkanMgr::getQueueFamily(),
            nullptr,
            w,
            h,
            VulkanMgr::getMinImageCount());

        ImGui_ImplSDL2_InitForVulkan(wSdl);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = VulkanMgr::getInst();
        initInfo.PhysicalDevice = VulkanMgr::getPhyDev();
        initInfo.Device = VulkanMgr::getDev();
        initInfo.QueueFamily = VulkanMgr::getQueueFamily();
        initInfo.Queue = VulkanMgr::getQueue();
        initInfo.PipelineCache = VulkanMgr::getPiplineCache();
        initInfo.DescriptorPool = VulkanMgr::getDescPool();
        initInfo.Subpass = 0;
        initInfo.MinImageCount = VulkanMgr::getMinImageCount();
        initInfo.ImageCount = _inst->wHandle.ImageCount;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = VulkanMgr::checkVkResultCtype;
        ImGui_ImplVulkan_Init(&initInfo, _inst->wHandle.RenderPass);
        initDefaultStyle();
        prePresentFont();
    }

    void ImguiMgr::render(const ImVec4& clearColor)
    {
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        const bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
        if (isMinimized) return;

        _inst->wHandle.ClearValue.color.float32[0] = clearColor.x * clearColor.w;
        _inst->wHandle.ClearValue.color.float32[1] = clearColor.y * clearColor.w;
        _inst->wHandle.ClearValue.color.float32[2] = clearColor.z * clearColor.w;
        _inst->wHandle.ClearValue.color.float32[3] = clearColor.w;

        VkResult err;
        VkSemaphore image_acquired_semaphore  = _inst->wHandle.FrameSemaphores[_inst->wHandle.SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore = _inst->wHandle.FrameSemaphores[_inst->wHandle.SemaphoreIndex].RenderCompleteSemaphore;
        err = vkAcquireNextImageKHR(VulkanMgr::getDev(), _inst->wHandle.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &(_inst->wHandle.FrameIndex));
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            _inst->swapChainRebuild = true;
            return;
        }
        VulkanMgr::checkVkResultCtype(err);

        ImGui_ImplVulkanH_Frame* fd = &_inst->wHandle.Frames[_inst->wHandle.FrameIndex];
        {
            err = vkWaitForFences(VulkanMgr::getDev(), 1, &fd->Fence, VK_TRUE, UINT64_MAX);
            VulkanMgr::checkVkResultCtype(err);

            err = vkResetFences(VulkanMgr::getDev(), 1, &fd->Fence);
            VulkanMgr::checkVkResultCtype(err);
        }
        {
            err = vkResetCommandPool(VulkanMgr::getDev(), fd->CommandPool, 0);
            VulkanMgr::checkVkResultCtype(err);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
            VulkanMgr::checkVkResultCtype(err);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = _inst->wHandle.RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = _inst->wHandle.Width;
            info.renderArea.extent.height = _inst->wHandle.Height;
            info.clearValueCount = 1;
            info.pClearValues = &_inst->wHandle.ClearValue;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        // 将 imgui 渲染流程写入指令缓冲
        ImGui_ImplVulkan_RenderDrawData(drawData, fd->CommandBuffer);

        // 为提呈指令缓冲做准备
        vkCmdEndRenderPass(fd->CommandBuffer);
        {
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &waitStage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete_semaphore;

            err = vkEndCommandBuffer(fd->CommandBuffer);
            VulkanMgr::checkVkResultCtype(err);
            err = vkQueueSubmit(VulkanMgr::getQueue(), 1, &info, fd->Fence);
            VulkanMgr::checkVkResultCtype(err);
        }

        // 提呈指令缓冲
        present();
    }

    void ImguiMgr::present()
    {
        if (_inst->swapChainRebuild) return;
        VkSemaphore render_complete_semaphore = _inst->wHandle.FrameSemaphores[_inst->wHandle.SemaphoreIndex].RenderCompleteSemaphore;
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &render_complete_semaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &_inst->wHandle.Swapchain;
        info.pImageIndices = &_inst->wHandle.FrameIndex;
        VkResult err = vkQueuePresentKHR(VulkanMgr::getQueue(), &info);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) { _inst->swapChainRebuild = true; return; }
        VulkanMgr::checkVkResultCtype(err);
        _inst->wHandle.SemaphoreIndex = (_inst->wHandle.SemaphoreIndex + 1) % _inst->wHandle.ImageCount;
    }

    void ImguiMgr::prePresentFont()
    {
        static const int nameMaxSize = 25;
        static const char* name = "source han san scn normal";
        auto io = CC::ImguiMgr::getIO();
        auto fontConfig = ImFontConfig();
        fontConfig.FontDataOwnedByAtlas = false;
        for (auto i = 0; i < nameMaxSize; i++)
            fontConfig.Name[i] = name[i];

        ImVector<ImWchar> ranges;
        ImFontGlyphRangesBuilder builder;

        builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        builder.AddText((const char*)CC::RC::CC_RC_FONTS_______________________UCC8105___1_0X00_.data());
        builder.BuildRanges(&ranges);

        io.Fonts->AddFontFromMemoryTTF(
            (void*)CC::RC::CC_RC_FONTS_SOURCEHANSANSCN_NORMAL_OTF.data(),
            CC::RC::CC_RC_FONTS_SOURCEHANSANSCN_NORMAL_OTF.size(),
            32.0f,
            &fontConfig,
            ranges.Data);
        io.Fonts->Build();

        VkCommandPool commandPool = _inst->wHandle.Frames[_inst->wHandle.FrameIndex].CommandPool;
        VkCommandBuffer commandBuffer = _inst->wHandle.Frames[_inst->wHandle.FrameIndex].CommandBuffer;

        auto err = vkResetCommandPool(VulkanMgr::getDev(), commandPool, 0);
        VulkanMgr::checkVkResultCtype(err);
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VulkanMgr::checkVkResultCtype(err);

        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

        VkSubmitInfo endInfo = {};
        endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        endInfo.commandBufferCount = 1;
        endInfo.pCommandBuffers = &commandBuffer;
        err = vkEndCommandBuffer(commandBuffer);
        VulkanMgr::checkVkResultCtype(err);
        err = vkQueueSubmit(VulkanMgr::getQueue(), 1, &endInfo, VK_NULL_HANDLE);
        VulkanMgr::checkVkResultCtype(err);

        err = vkDeviceWaitIdle(VulkanMgr::getDev());
        VulkanMgr::checkVkResultCtype(err);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void ImguiMgr::presentFont()
    {
        // 还没有顺利找到 imgui 的动态字体解决方案, 先推进别的

        // static const int nameMaxSize = 25;
        // static const char* name = "source han san scn normal";
        // auto io = CC::ImguiMgr::getIO();
        // auto fontConfig = ImFontConfig();
        // fontConfig.FontDataOwnedByAtlas = false;
        // for (auto i = 0; i < nameMaxSize; i++)
        //     fontConfig.Name[i] = name[i];

        // io.Fonts->AddFontFromMemoryTTF(
        //     (void*)CC::RC::CC_RC_FONTS_SOURCEHANSANSCN_NORMAL_OTF.data(),
        //     CC::RC::CC_RC_FONTS_SOURCEHANSANSCN_NORMAL_OTF.size(),
        //     32.0f,
        //     &fontConfig,
        //     io.Fonts->GetGlyphRangesChineseFull());
        // io.Fonts->Build();

        // VkCommandPool commandPool = _inst->wHandle.Frames[_inst->wHandle.FrameIndex].CommandPool;
        // VkCommandBuffer commandBuffer = _inst->wHandle.Frames[_inst->wHandle.FrameIndex].CommandBuffer;

        // auto err = vkResetCommandPool(VulkanMgr::getDev(), commandPool, 0);
        // VulkanMgr::checkVkResultCtype(err);
        // VkCommandBufferBeginInfo beginInfo = {};
        // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        // err = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        // VulkanMgr::checkVkResultCtype(err);

        // ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

        // VkSubmitInfo endInfo = {};
        // endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        // endInfo.commandBufferCount = 1;
        // endInfo.pCommandBuffers = &commandBuffer;
        // err = vkEndCommandBuffer(commandBuffer);
        // VulkanMgr::checkVkResultCtype(err);
        // err = vkQueueSubmit(VulkanMgr::getQueue(), 1, &endInfo, VK_NULL_HANDLE);
        // VulkanMgr::checkVkResultCtype(err);

        // err = vkDeviceWaitIdle(VulkanMgr::getDev());
        // VulkanMgr::checkVkResultCtype(err);
        // ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void ImguiMgr::initDefaultStyle()
    {
        // --------------------------------------------------- 设置样式
        ImGui::StyleColorsLight();
        auto& styleContext = ImGui::GetStyle();

        // --------------------------------------------------- 设置颜色
        ImVec4* colors = styleContext.Colors;
        colors[ImGuiCol_Text]                   = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.93f, 0.93f, 0.89f, 0.98f);
        colors[ImGuiCol_Border]                 = ImVec4(0.27f, 0.10f, 0.72f, 0.30f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.62f, 0.20f, 0.17f, 0.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.67f, 0.82f, 1.00f, 0.40f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.16f, 0.07f, 0.23f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.84f, 0.73f, 0.85f, 0.80f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.74f, 0.79f, 0.93f, 0.80f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(1.00f, 0.80f, 0.80f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 0.17f, 0.17f, 0.78f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.96f, 0.84f, 0.36f, 0.60f);
        colors[ImGuiCol_Button]                 = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.75f, 0.92f, 0.88f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.51f, 0.91f, 0.67f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.78f, 0.93f, 1.00f, 1.00f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.60f, 0.78f, 0.87f, 1.00f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.75f, 0.86f, 0.91f, 1.00f);
        colors[ImGuiCol_Tab]                    = ImVec4(1.00f, 1.00f, 0.87f, 0.93f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.93f, 0.80f, 0.50f, 0.80f);
        colors[ImGuiCol_TabActive]              = ImVec4(1.00f, 0.54f, 0.54f, 1.00f);

        // --------------------------------------------------- 设置排版参数
        styleContext.WindowPadding = {10, 10};
        styleContext.FramePadding = {5, 2};
        styleContext.CellPadding = {5, 5};
        styleContext.ItemSpacing = {6, 6};
        styleContext.ItemInnerSpacing = {4, 4};
        styleContext.TouchExtraPadding = {2, 2};
        styleContext.IndentSpacing = 20;
        styleContext.ScrollbarSize = 14;
        styleContext.GrabMinSize = 18;

        styleContext.WindowBorderSize = 1;
        styleContext.ChildBorderSize = 1;
        styleContext.PopupBorderSize = 0;
        styleContext.FrameBorderSize = 1;
        styleContext.TabBorderSize = 0;

        styleContext.WindowRounding = 0;
        styleContext.ChildRounding = 5;
        styleContext.FrameRounding = 2;
        styleContext.PopupRounding = 0;
        styleContext.ScrollbarRounding = 2;
        styleContext.GrabRounding = 1;
        styleContext.LogSliderDeadzone = 8;
        styleContext.TabRounding = 3;

        styleContext.WindowTitleAlign = {0.5f, 0.5f};
        styleContext.WindowMenuButtonPosition = ImGuiDir_Right;
        styleContext.ColorButtonPosition = ImGuiDir_Left;
        styleContext.ButtonTextAlign = {0.5f, 0.5f};
        styleContext.SelectableTextAlign = {0, 0};
        styleContext.DisplaySafeAreaPadding = {4, 3};

        getIO().FontGlobalScale = 0.78125f;
        getIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    void ImguiMgr::disableLoading()
    {
        _inst->notloadingRendered = false;
        _inst->notLoaded = false;
    }

    void ImguiMgr::finishLoading()
    {
        _inst->notloadingRendered = false;
    }

    void ImguiMgr::preRender(SDL_Window* window, int& w, int& h)
    {
        if (_inst->notLoaded && !_inst->notloadingRendered)
        {
            presentFont();
            _inst->notLoaded = false;
        }

        if (_inst->swapChainRebuild)
        {
            int width, height;
            SDL_GetWindowSize(window, &width, &height);
            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(VulkanMgr::getMinImageCount());
                ImGui_ImplVulkanH_CreateOrResizeWindow(
                    VulkanMgr::getInst(),
                    VulkanMgr::getPhyDev(),
                    VulkanMgr::getDev(),
                    &(_inst->wHandle),
                    VulkanMgr::getQueueFamily(),
                    nullptr,
                    width,
                    height,
                    VulkanMgr::getMinImageCount());

                _inst->wHandle.FrameIndex = 0;
                _inst->swapChainRebuild = false;
            }
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void ImguiMgr::destroy()
    {
        ImGui_ImplVulkanH_DestroyWindow(VulkanMgr::getInst(), VulkanMgr::getDev(), &(_inst->wHandle), nullptr);
        _inst.reset();
    }
}