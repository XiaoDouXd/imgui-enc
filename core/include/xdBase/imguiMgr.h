#pragma once

#include <memory>
#include <SDL.h>

#include "ccexce.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

namespace XD::ImguiMgr {

    /// @brief is imgui inited
    bool inited();

    /// @brief is font loading finish (while loading async enable)
    void finishLoading();

    /// @brief is disable loading async
    void disableLoading();

    /// @brief is loading finish (while loading async enable)
    bool notLoadingFinished();

    /// @brief called while before render a frame
    void preRender(SDL_Window* window, [[maybe_unused]] int& w, [[maybe_unused]] int& h);

    /// @brief called while rendering a frame
    void render(const ImVec4& clearColor);

    /// @brief get imgui handle
    ImGui_ImplVulkanH_Window& getHWnd();

    /// @brief get imgui io handle
    ImGuiIO& getIO();

    /// @brief get imgui context
    ImGuiContext& getContext();
}
#pragma clang diagnostic pop