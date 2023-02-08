#pragma once

#include "base/wndBase.hpp"
#include "entrance.h"
#include "ctrlPanel.hpp"
#include "picPanel.hpp"

namespace CC::UI
{
    static size_t ctrlPanelId = 0;
    static size_t picPanelId = 0;

    static void openCtrlPanel()
    {
        if (ctrlPanelId == 0)
        {
            auto data = new WndDataDefault();
            data->onHideCB.emplace_back([](WndBaseHolder& wnd){ ctrlPanelId = 0; });
            ctrlPanelId = WndMgr::open<CtrlPanel>(data);
        }
    }

    static void openPicPanel()
    {
        if (picPanelId == 0)
        {
            auto data = new WndDataDefault();
            data->onHideCB.emplace_back([](WndBaseHolder& wnd)
            {
                picPanelId = 0;
                App::logInfo("CC::MainPanel log - Failure to add file\n");
            });
            picPanelId = WndMgr::open<PicPanel>(data);
        }
    }

    class MainPanel : public WndBase<MainPanel>
    {
    private:
        ImGuiWindowFlags bgWindowFlags  = 0;
        ImGuiWindowFlags windowFlags    = 0;
        ImGuiDockNodeFlags dockFlags    = 0;
        bool open                       = true;
        ImDrawList* drawList            = nullptr;
    protected:
        void onShow(WndDataBaseHolder* ) override
        {
            bgWindowFlags |= ImGuiWindowFlags_NoResize;
            bgWindowFlags |= ImGuiWindowFlags_NoMove;
            bgWindowFlags |= ImGuiWindowFlags_NoScrollbar;
            bgWindowFlags |= ImGuiWindowFlags_NoNav;
            bgWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
            bgWindowFlags |= ImGuiWindowFlags_NoCollapse;
            bgWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;
            bgWindowFlags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;

            windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
            windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav;
            windowFlags |= ImGuiWindowFlags_NoDocking;

            dockFlags;

            onRegister();
        };

        void onRefresh() override
        {
            docking();
        };

        bool onWndBegin() override
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, (ImU32)ImColor(1.f, 1.f, 1.f, 0.f));
            if (ImGui::Begin("mainPanel", &open, windowFlags)) return true;
            return false;
        }

        void onWndEnd() override
        {
            ImGui::End();
            ImGui::PopStyleVar(4);
            ImGui::PopStyleColor();
            drawGridWnd();
        }

    private:
        // --------------------------- 网格背景
        /// @brief 绘制背景及网格
        void drawGridWnd()
        {
            if (ImGui::Begin("mainPanel_bg", &open, bgWindowFlags))
            {
                ImGui::SetWindowSize({(float)App::getW() + 128, (float)App::getH() + 128});
                ImGui::SetWindowPos({-64, -64});
                static ImVec2 scrolling(0.0f, 0.0f);

                ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
                if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
                ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

                ImGuiIO& io = ImGui::GetIO();
                drawList = ImGui::GetWindowDrawList();

                const bool is_active = ImGui::IsItemActive();
                const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y);
                const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

                const float mouse_threshold_for_pan = 0.0f;
                if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, mouse_threshold_for_pan) && !App::event().isWindowResizing)
                {
                    scrolling.x += io.MouseDelta.x;
                    scrolling.y += io.MouseDelta.y;
                }

                const float GRID_STEP = 64.0f;
                for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
                    drawList->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
                for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
                    drawList->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
            }
            ImGui::End();
        }

        // --------------------------- 刷新窗口停靠
        void docking()
        {
            ImGui::DockSpace(ImGui::GetID("mainPanel_docking"), {0.f, 0.f}, dockFlags);
        }

        // ---------------------------- 绑定事件
        void onRegister()
        {
            registerEvent<StaticEvent::OnFilePush>([this](){this->onOpenFile();});
        }

        void onOpenFile()
        {
            openCtrlPanel();
            openPicPanel();
        }
    };
}