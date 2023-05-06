#pragma once

#include <memory>
#include <SDL.h>

#include "ccexce.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"

namespace XD::ImguiMgr {
    bool inited();
    void finishLoading();
    void disableLoading();
    bool notLoadingFinished();
    void preRender(SDL_Window* window, [[maybe_unused]] int& w, [[maybe_unused]] int& h);
    void render(const ImVec4& clearColor);

    ImGui_ImplVulkanH_Window& getHWnd();
    ImGuiIO& getIO();
    ImGuiContext& getContext();
}