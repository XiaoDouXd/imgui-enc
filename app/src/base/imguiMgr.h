#pragma once

#include <memory>
#include <SDL.h>

#include "ccexce.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"

namespace CC
{
#define CHECK_INITED if (!inited()) throw Exce(__LINE__, __FILE__, "CC::ImgMgr Exce: not init")
    class ImguiMgr
    {
        friend class App;
    private:
        ImguiMgr() {}
        class ImguiMgrData
        {
        public:
            ImGui_ImplVulkanH_Window    wHandle;
            ImGuiIO*                    io                  = nullptr;
            ImGuiContext*               context             = nullptr;
            bool                        swapChainRebuild    = false;
            bool                        notloadingRendered  = true;
            bool                        notLoaded           = true;
        };
        static std::unique_ptr<ImguiMgrData> _inst;

    public:
        static bool inited();
        static void finishLoading();
        static void disableLoading();
        static bool notLoadingFinished();
        static void preRender(SDL_Window* window, int& w, int& h);
        static void render(const ImVec4& clearColor);

    public:
        static ImGui_ImplVulkanH_Window& getHWnd() { CHECK_INITED; return _inst->wHandle; }
        static ImGuiIO& getIO() { CHECK_INITED; return *(_inst->io); }
        static ImGuiContext& getContext() { CHECK_INITED; return *(_inst->context); }

    private:
        static void init(SDL_Window* wSdl, VkSurfaceKHR& surf, int w, int h);
        static void destroy();

        static void presentFont();
        static void prePresentFont();
        static void present();
        static void initDefaultStyle();
    };
#undef CHECK_INITED
}