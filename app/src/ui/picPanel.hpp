#pragma once

#include <string>

#include "core/clipCtrl.h"
#include "core/fileCtrl.h"
#include "base/wndBase.hpp"
#include "math.h"
#include "unit/imgUnit.h"

#include "imgui_internal.h"

namespace CC::UI
{
    const ImageUnit::ImageUnitDesc ImageUnit::_emptyImgUnitDesc = {};

    /// @brief 主界面 UI
    class PicPanel : public WndBase<PicPanel>
    {
    public:
        void pushPic(std::filesystem::path path)
        {
            try
            {
                _imgs.emplace_back(path);
            }
            catch(const std::exception& e) { }
            refreshData();
        }

    private:
        class Pic
        {
        public:
            Pic(std::filesystem::path path) : unit(path) {}
            ImVec2 pos = {};
            ImageUnit unit;
            const ImageUnit::ImageUnitDesc& desc() const { return unit.desc(); }
            const int& w() const { return unit.desc().w; }
            const int& h() const { return unit.desc().h; }
            ImVec2 size() const { return {(float)w(), (float)h()}; }
        };

        // ------------------- 窗口设置

        ImGuiWindowFlags windowFlags    = 0;
        bool _refreshP0                 = true;

        // ------------------- 图像绘制相关
        bool _isMouseInVp               = false;
        ImVec2 _p0                      = {};
        ImVec2 _focus                   = {};
        ImVec2 _min                     = {};
        ImVec2 _max                     = {};
        ImVec2 _aabbRect                = {};
        float _s                        = 1.f;
        float _alphaRectWid             = 16.0f;
        std::list<Pic> _imgs;

        // ------------------- 背景绘制相关
        ImDrawList* _drawList      = nullptr;
        ImVec2 _wMax               = {};
        ImVec2 _wPos               = {};
        ImVec2 _wSize              = {};
        ImVec2 _wMousePos          = {};
        std::string _posStr        = {};

    protected:
        void onShow(WndDataBaseHolder*) override
        {
            try
            {
                _imgs.emplace_back(FileCtrl::getInst().fileQueue.front());
            }
            catch(const std::exception& e)
            {
                closeSelf();
                _imgs.clear();
                return;
            }

            _refreshP0 = true;
            refreshData();
            WndMgr::open<CtrlPanel>();
            registerEvent<StaticEvent::OnShortcut>([this](Shortcut k){this->onShortcut(k);});
        }

        void onRefresh() override
        {
            _drawList->AddText({_wPos.x + 4.f, _wMax.y - 26}, IM_COL32_BLACK, _posStr.c_str());
            ImGui::PushClipRect(_wPos, {_wMax.x, _wMax.y - 26}, true);
            ctrlScale();
            drawAlphaBg();
            drawImg();
            drawClips();
            ImGui::PopClipRect();
        }

        bool onWndBegin() override
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            if (ImGui::Begin("队顶文件", nullptr, windowFlags))
            {
                _wPos = {
                    ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x,
                    ImGui::GetWindowContentRegionMin().y + ImGui::GetWindowPos().y
                };
                _wSize = {
                    ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
                    ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y
                };
                _wMax = {
                    ImGui::GetWindowContentRegionMax().x + ImGui::GetWindowPos().x,
                    ImGui::GetWindowContentRegionMax().y + ImGui::GetWindowPos().y
                };

                _wMousePos = {
                    ImGui::GetMousePos().x - _wPos.x,
                    ImGui::GetMousePos().y - _wPos.y
                };

                if (_refreshP0)
                {
                    _p0 = {
                        _wSize.x * 0.5f - scale(_imgs.front().desc().w) * 0.5f,
                        _wSize.y * 0.5f - scale(_imgs.front().desc().h) * 0.5f
                    };
                    _s = 1.f;
                    _refreshP0 = false;
                }

                _posStr = "| (" +
                    std::to_string((int)((_wMousePos.x - _p0.x) / _s)) + ", " +
                    std::to_string((int)((_wMousePos.y - _p0.y) / _s)) + ") |";

                _drawList = ImGui::GetWindowDrawList();
                return true;
            }
            return false;
        }

        void onWndEnd() override
        {
            ImGui::End();
            ImGui::PopStyleVar();
        }

    private:
        // ---------------------------------------------------------------- 控制
        void ctrlScale()
        {
            _isMouseInVp = _wMousePos.x >= 0 &&
                _wMousePos.x <= _wSize.x &&
                _wMousePos.y >= 0 &&
                _wMousePos.y <= _wSize.y;
            if (_isMouseInVp && !ImGui::IsWindowDocked()) windowFlags |= ImGuiWindowFlags_NoMove;
            else windowFlags &= !ImGuiWindowFlags_NoMove;

            if (ImGui::IsWindowFocused() && ImguiMgr::getIO().MouseWheel != 0)
            {
                auto _sOld = _s;
                _s += ImguiMgr::getIO().MouseWheel * (_s * 0.16f);
                if (_s <= 0.05f) _s = 0.05f;
                if (_s > 36.f) _s = 36.f;

                if (_isMouseInVp)
                {
                    _p0.x += (_wMousePos.x * _sOld - _wMousePos.x * _s) / 2;
                    _p0.y += (_wMousePos.y * _sOld - _wMousePos.y * _s) / 2;
                }
                else
                {
                    _p0.x += (_focus.x * _sOld - _focus.x * _s) / 2;
                    _p0.y += (_focus.y * _sOld - _focus.y * _s) / 2;
                }
            }

            if (_isMouseInVp && ImGui::IsWindowFocused())
            {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::IsWindowCollapsed())
                {
                    auto dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                    _p0.x += ImguiMgr::getIO().MouseDelta.x;
                    _p0.y += ImguiMgr::getIO().MouseDelta.y;

                    if (_p0.x < -2 * _aabbRect.x * _s) _p0.x = -2 * _aabbRect.x * _s;
                    if (_p0.y < -2 * _aabbRect.y * _s) _p0.y = -2 * _aabbRect.y * _s;
                    if (_p0.x > _aabbRect.x * _s + _wSize.x) _p0.x = _aabbRect.x * _s + _wSize.x;
                    if (_p0.y > _aabbRect.y * _s + _wSize.y) _p0.y = _aabbRect.y * _s + _wSize.y;
                }
            }
        }

        void onShortcut(Shortcut k)
        {
            switch (k)
            {
            case Shortcut::Undo:
                App::logInfo("CC::PicPanel info: 撤回撤回!!\n");
                ClipCtrl::undo();
                return;
            case Shortcut::Redo:
                App::logInfo("CC::PicPanel info: 重做重做!!\n");
                ClipCtrl::redo();
                return;
            default: return;
            }
        }

        // ---------------------------------------------------------------- 绘制
        /// @brief 绘制 image
        void drawImg()
        {
            for (const auto&i : _imgs)
            {
                addImg(i.unit.getId(), i.pos, i.size());
                addRect(i.pos, i.size(), IM_COL32_BLACK);
            }
        }

        void drawClips()
        {
            for (auto& i : ClipCtrl::getCurClips())
                addRect(i.min, i.size, ImColor(1.f, 0.f, 0.f));
        }

        void drawAlphaBg()
        {
            auto rectWid_scale = scale(_alphaRectWid);
            while (rectWid_scale < 8.f) rectWid_scale *= 2;

            for (auto i = _p0.x; i < rectWid_scale + _wSize.x; i += rectWid_scale * 2)
            {
                for (auto j = _p0.y; j < rectWid_scale + _wSize.y; j += rectWid_scale * 2)
                    addAlphaRect({i, j}, rectWid_scale);
                for (auto j = _p0.y - 2*rectWid_scale; j > -rectWid_scale; j -= rectWid_scale * 2)
                    addAlphaRect({i, j}, rectWid_scale);
            }
            for (auto i = _p0.x - 2*rectWid_scale; i > -rectWid_scale; i -= rectWid_scale * 2)
            {
                for (auto j = _p0.y; j < rectWid_scale + _wSize.y; j += rectWid_scale * 2)
                    addAlphaRect({i, j}, rectWid_scale);
                for (auto j = _p0.y - 2*rectWid_scale; j > -rectWid_scale; j -= rectWid_scale * 2)
                    addAlphaRect({i, j}, rectWid_scale);
            }
        }

        void addImg(ImTextureID imgId, ImVec2 pos, ImVec2 size)
        {
            _drawList->AddImage(imgId,
                {_wPos.x + _p0.x + scale(pos.x), _wPos.y + _p0.y + scale(pos.y)},
                {_wPos.x + _p0.x + scale(size.x + pos.x), _wPos.y + _p0.y + scale(size.y + pos.y)});
        }

        void addRect(ImVec2 pos, ImVec2 size, ImColor col)
        {
            _drawList->AddRect(
                {_wPos.x + _p0.x + scale(pos.x), _wPos.y + _p0.y + scale(pos.y)},
                {_wPos.x + _p0.x + scale(size.x + pos.x), _wPos.y + _p0.y + scale(size.y + pos.y)},
                col);
        }

        void addRectFilled(ImVec2 pos, ImVec2 size, ImColor col)
        {
            _drawList->AddRectFilled(
                {_wPos.x + _p0.x + pos.x, _wPos.y + _p0.y + pos.y},
                {_wPos.x + size.x + _p0.x + pos.x, _wPos.y + size.y + _p0.y + pos.y},
                col);
        }

        void addAlphaRect(ImVec2 pos, float size)
        {
            _drawList->AddRectFilled(
                {_wPos.x + pos.x, _wPos.y + pos.y},
                {_wPos.x + size + pos.x, _wPos.y + size + pos.y},
                ImColor(0.f, 0.f, 0.f, 0.25f));
            _drawList->AddRectFilled(
                {_wPos.x + pos.x - size, _wPos.y + pos.y - size},
                {_wPos.x  + pos.x, _wPos.y + pos.y},
                ImColor(0.f, 0.f, 0.f, 0.25f));
        }

        // ---------------------------------------------------------------- 计算
        float scale(float src)
        {
            return src * _s;
        }

        void refreshData()
        {
            ImVec2 f = {};
            ImVec2 mi = {_imgs.front().pos.x, _imgs.front().pos.y};
            ImVec2 ma = {_imgs.front().pos.x, _imgs.front().pos.y};
            for (const auto& i : _imgs)
            {
                mi.x = std::min({i.pos.x, mi.x});
                mi.y = std::min({i.pos.y, mi.y});

                ma.x = std::max({i.pos.x + i.desc().w, ma.x});
                ma.y = std::max({i.pos.y + i.desc().h, ma.y});

                f.x += i.desc().w;
                f.y += i.desc().h;
            }
            f.x /= _imgs.size();
            f.y /= _imgs.size();
            _focus = f;
            _min = mi;
            _max = ma;

            _aabbRect.x = _max.x - _min.x;
            _aabbRect.y = _max.y - _min.y;
        }
    };
}