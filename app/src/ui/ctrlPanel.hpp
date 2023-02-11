#pragma once

#include <cmath>

#include "misc/cpp/imgui_stdlib.h"

#include "base/wndBase.hpp"
#include "core/clipCtrl.h"
#include "entrance.h"
#include "util/rectTest.hpp"

namespace CC::UI
{
    const char* gSizeLabel = "- 网格宽高";
    const char* gOffsetLabel = "- 网格偏移";
    const char* gCountLabel = "- 网格行列数量";
    const char* createGrid = "建立网格";
    const char* createGridOK = "确定";
    const ImGuiWindowFlags popFlag =    ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoDocking;

    class CtrlPanel : public WndBase<CtrlPanel>
    {
    private:
        // ------------------- 窗口设置

        ImGuiWindowFlags windowFlags    = 0;
        ImVec4 clearColor               = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

        int rectOffset[2]               = {0, 0};
        int rectWidth[2]                = {32, 32};
        int rectCount[2]                = {2, 2};

        // ------------------- 网格窗口
        bool createGridPopShow              = false;

    protected:
        void onShow(WndDataBaseHolder*) override
        {
            windowFlags |= ImGuiWindowFlags_NoCollapse;
        };

        void onRefresh() override
        {
            if (ImGui::BeginTabItem("生成"))
            {
                // static float f = 0.0f;
                // ImGui::Text("行文本.");                                 // Display some text (you can use a format strings too)
                // ImGui::InputInt2(gSizeLabel, rectWidth);

                if (ImGui::Button(createGrid))
                    createGridPopShow = true;

                // ImGui::SliderFloat("浮点数", &f, 0.0f, 1.0f);           // Edit 1 float using a slider from 0.0f to 1.0f
                // ImGui::ColorEdit3("背景色", (float*)&clearColor);       // Edit 3 floats representing a color
                // ImGui::SameLine();

                ImGui::Text("程序刷新率 %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::EndTabItem();
            }
            ClipList();
            ImGui::EndTabBar();

            // ------------------------- 一些弹窗
            createGridPop();
        }

        bool onWndBegin() override
        {
            if (ImGui::Begin("控制台", nullptr, windowFlags))
            {
                ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None);
                return true;
            }
            return false;
        }

        void onWndEnd() override
        {
            ImGui::End();
        }
    private:
        void createGridPop()
        {
            if (!createGridPopShow) return;
            ImGui::Begin("创建网格切片", &createGridPopShow, popFlag);

            ImGui::InputInt2(gSizeLabel, rectWidth);
            ImGui::InputInt2(gOffsetLabel, rectOffset);
            ImGui::InputInt2(gCountLabel, rectCount);
            if (ImGui::Button(createGridOK))
            {
                if (rectCount[0]  <= 0 || rectCount[1] <= 0)
                {
                    rectCount[0] = 0;
                    rectCount[1] = 0;
                }
                createGridPopShow = false;
                auto rects = std::vector<Clip>(rectCount[0] * rectCount[1]);
                auto curIdx = 0;
                for (auto i = 0; i < rectCount[0]; i++)
                {
                    for (auto j = 0; j < rectCount[1]; j++)
                    {
                        rects[curIdx] = Clip(
                            {(float)(i * rectWidth[0] + rectOffset[0]), (float)(j * rectWidth[1] + rectOffset[1])},
                            {(float)rectWidth[0], (float)rectWidth[1]});
                        curIdx++;
                    }
                }
                ClipCtrl::push(rects);
            }

            ImGui::End();
        }

        // -------------------------------------------- 切片列表
        bool _clipList_startDrag                        = false;
        bool _clipList_startDragAndListOperationEnabled = false;
        bool _clipList_enableListOperation              = false;
        int _clipList_dragStart                         = 0;
        int _clipList_selectItem_gridList               = 0;
        void ClipList()
        {
            if (ImGui::BeginTabItem("切片列表"))
            {
                ImGui::BeginListBox("切片", ImVec2(-FLT_MIN, -FLT_MIN));
                auto i = size_t(0);
                std::string name;
                ImGuiStyle& style = ImGui::GetStyle();

                ImVec2 windowPosition = ImGui::GetWindowPos();
                ImVec2 cursorPosition = ImGui::GetCursorPos();

                ImVec2 itemPosition (
                    windowPosition.x + cursorPosition.x,
                    windowPosition.y + cursorPosition.y - style.ItemSpacing.y
                );

                const auto& clips = ClipCtrl::getCurClips();
                for (int i = 0; i < clips.size(); i++)
                {
                    name = "[" + std::to_string(i) + "] " + (std::string)clips[i];
                    ImGui::Selectable(name.c_str());
                    if (ImGui::IsWindowFocused() && _clipList_enableListOperation &&
                        rectTest(ImGui::GetMousePos(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
                    {
                        _clipList_selectItem_gridList = i;
                        if (_clipList_startDragAndListOperationEnabled)
                        {
                            ImGui::GetWindowDrawList()->AddRectFilled(
                                ImGui::GetItemRectMin(),
                                ImGui::GetItemRectMax(),
                                ImColor(1.f, 0.2f, 0.f, 0.25f));
                        }
                    }

                    if (ImGui::IsWindowFocused())
                    {
                        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                        {
                            if (!_clipList_startDrag)
                            {
                                _clipList_startDragAndListOperationEnabled = _clipList_enableListOperation;
                                _clipList_startDrag = true;
                                if (_clipList_selectItem_gridList >= 0 && _clipList_selectItem_gridList < clips.size())
                                    _clipList_dragStart = _clipList_selectItem_gridList;
                            }
                        }
                        else if (_clipList_startDrag)
                        {
                            if (_clipList_startDragAndListOperationEnabled)
                            {
                                int n_next = _clipList_selectItem_gridList;
                                if (n_next != _clipList_dragStart && n_next >= 0 && n_next < clips.size())
                                    ClipCtrl::swap(_clipList_dragStart, n_next);
                            }
                            _clipList_startDrag = false;
                            _clipList_startDragAndListOperationEnabled = false;
                        }
                    }
                    ImGui::Separator();
                }
                ImGui::EndListBox();
                if (rectTest(ImGui::GetMousePos(),
                    ImGui::GetItemRectMin(),
                    {ImGui::GetItemRectMax().x - 50, ImGui::GetItemRectMax().y}))
                    _clipList_enableListOperation = true;
                else
                {
                    _clipList_enableListOperation = false;
                    _clipList_dragStart           = 0;
                    _clipList_selectItem_gridList = 0;
                }
                ImGui::EndTabItem();
            }
        }
    };
}