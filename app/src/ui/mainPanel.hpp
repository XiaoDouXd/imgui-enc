#include <iostream>
#include <string>

#include "base/wndBase.hpp"
#include "math.h"
#include "unit/imgUnit.hpp"

namespace CC::UI
{
    /// @brief 主界面 UI
    class MainPanel : public WndBase<MainPanel>
    {
    private:
        // ------------------- 窗口设置

        ImGuiWindowFlags windowFlags    = 0;
        bool open                       = true;

        // ------------------- 图像绘制相关
        std::unique_ptr<ImageUnit> Img  = nullptr;

        // ------------------- 背景绘制相关

        ImDrawList* drawList            = nullptr;
        ImVec4 clearColor               = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    protected:
        void onShow(WndDataBaseHolder*) override
        {
            windowFlags |= ImGuiWindowFlags_NoResize;
            windowFlags |= ImGuiWindowFlags_NoMove;
            windowFlags |= ImGuiWindowFlags_NoScrollbar;
            windowFlags |= ImGuiWindowFlags_NoNav;
            windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
            windowFlags |= ImGuiWindowFlags_NoCollapse;
            windowFlags |= ImGuiWindowFlags_NoTitleBar;
            windowFlags |= ImGuiWindowFlags_NoBackground;

            Img = std::make_unique<ImageUnit>("./i512.png");
        }

        void onRefresh() override
        {
            if (!mainPanelBegin()) return; {
                drawGrid();
                drawImg();
            }
            mainPanelEnd();

            // ---- 临时
            drawTempWnd();
        }

        void onHide() override {}

    private:
        // ----------------------------------------------------------------

        /// @brief 绘制头
        /// @return 窗口是否存在
        bool mainPanelBegin()
        {
            if (!open) App::quit();
            if (ImGui::Begin("405c77b4-0cb6-4488-86d1-cab26266de9e", &open, windowFlags)) return true;
            ImGui::End();
            return false;
        }

        /// @brief 绘制尾
        void mainPanelEnd()
        {
            ImGui::End();
        }

        // ----------------------------------------------------------------

        /// @brief 背景及网格
        void drawGrid()
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

            CC::App::setBGColor(clearColor);
        }

        void drawImg()
        {
            if (drawList)
            {
                drawList->AddImage(Img->getId(), {0, 0}, {10, 10});
            }
        }

        /// @brief 绘制临时窗口
        void drawTempWnd()
        {
            // 临时
            {
                static float f = 0.0f;

                ImGui::Begin("测试用临时窗口");                          // Create a window called "Hello, world!" and append into it.
                ImGui::Text("行文本.");                                 // Display some text (you can use a format strings too)

                ImGui::SliderFloat("浮点数", &f, 0.0f, 1.0f);           // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("背景色", (float*)&clearColor);       // Edit 3 floats representing a color

                if (ImGui::Button("关闭窗口"))                          // Buttons return true when clicked (most widgets return true when edited/activated)
                    App::quit();
                ImGui::SameLine();

                ImGui::Text("程序刷新率 %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }
        }
    };
}