#pragma once

#include <string>

#include "core/clipCtrl.h"
#include "core/fileCtrl.h"
#include "base/wndBase.hpp"
#include "math.h"
#include "unit/imgUnit.h"
#include "ctrlPanel.hpp"

#include "imgui_internal.h"

#include "staticPanelData.h"

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
            glm::ivec2 pos = {};
            ImageUnit unit;
            const ImageUnit::ImageUnitDesc& desc() const { return unit.desc(); }
            const int& w() const { return unit.desc().w; }
            const int& h() const { return unit.desc().h; }
            glm::ivec2 size() const { return {(float)w(), (float)h()}; }
        };

        // ------------------- 窗口设置

        ImGuiWindowFlags windowFlags    = 0;
        bool _refreshP0                 = true;

        // ------------------- 图像绘制相关
        bool _isMouseInVp               = false;
        glm::ivec2 _p0                  = {};
        glm::vec2 _focus                = {};
        glm::ivec2 _min                 = {};
        glm::ivec2 _max                 = {};
        glm::ivec2 _aabbRect            = {};
        float _s                        = 1.f;
        float _alphaRectWid             = 16.0f;
        std::list<Pic> _imgs;

        // ------------------- 选择框
        glm::ivec4 _selectBox           = {};
        bool _selectStart               = true;

        // ------------------- 背景绘制相关
        ImDrawList* _drawList      = nullptr;
        glm::ivec2 _wMax               = {};
        glm::ivec2 _wPos               = {};
        glm::ivec2 _wSize              = {};
        glm::ivec2 _wMousePos          = {};
        glm::ivec2 _cMousePos          = {};
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
            ImGui::PushClipRect({_wPos.x, _wPos.y}, {_wMax.x, _wMax.y - 26}, true);
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

                _cMousePos = {
                    (_wMousePos.x - _p0.x) / _s,
                    (_wMousePos.y - _p0.y) / _s
                };

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

            if (ImGui::IsWindowFocused() && rectTest({ImGui::GetMousePos().x, ImGui::GetMousePos().y}, _wPos, _wMax))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    curSelectedClipsReset = true;
                if (ImguiMgr::getIO().KeyCtrl && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    for (size_t i = 0; i < ClipCtrl::getCurClips().size(); i++)
                    {
                        if (i >= curSelectedClips.size()) break;
                        if (ClipCtrl::getCurClips()[i].test(_cMousePos))
                            curSelectedClips[i] = !curSelectedClips[i];
                    }
                }
                if (ImguiMgr::getIO().KeyCtrl && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    if (!_selectStart) {_selectBox.x = _cMousePos.x; _selectBox.y = _cMousePos.y;}
                    _selectBox.z = _cMousePos.x; _selectBox.w = _cMousePos.y;
                    _selectStart = true;
                }
                else
                {
                    _selectStart = false;
                }

                if (_selectStart)
                {
                    for (size_t i = 0; i < ClipCtrl::getCurClips().size(); i++)
                    {
                        if (i >= curSelectedClips.size()) break;
                        curSelectedClips[i] |= ClipCtrl::getCurClips()[i].test(_selectBox);
                    }
                }

                if (ImguiMgr::getIO().MouseWheel != 0)
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
            }

            if (_isMouseInVp && ImGui::IsWindowFocused())
            {
                if (!ImguiMgr::getIO().KeyCtrl &&
                    ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
                    !ImGui::IsWindowCollapsed())
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
            case Shortcut::Undo: ClipCtrl::undo();
                curSelectedClipsReset = true;
                return;
            case Shortcut::Redo: ClipCtrl::redo();
                curSelectedClipsReset = true;
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
            const auto& clips = ClipCtrl::getCurClips();
            for (auto i = size_t(0); i < clips.size(); i++)
                addClip(i);
            if (_selectStart)
                addRect({_selectBox.x, _selectBox.y}, {_selectBox.z - _selectBox.x, _selectBox.w - _selectBox.y}, IM_COL32_BLACK);
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

        // ---------------------------------------------------------------- 绘制 unit
        void addImg(ImTextureID imgId, glm::ivec2 pos, glm::ivec2 size)
        {
            _drawList->AddImage(imgId,
                {_wPos.x + _p0.x + scale(pos.x), _wPos.y + _p0.y + scale(pos.y)},
                {_wPos.x + _p0.x + scale(size.x + pos.x), _wPos.y + _p0.y + scale(size.y + pos.y)});
        }

        void addClip(size_t idx)
        {
            const auto& clip = ClipCtrl::getCurClips()[idx];
            if (clip.empty) return;
            if (idx == curHoveredClip)
                clip.traverse([this](const Clip& clip){
                    this->addRectFilled(clip.min, clip.size, ImColor(0.f, 0.f, 1.f, 0.2f));
                });
            if (idx == curDragedClip)
                clip.traverse([this](const Clip& clip){
                    this->addRectFilled(clip.min, clip.size, ImColor(1.f, 0.f, 0.f, 0.2f));
                });
            if (idx < curSelectedClips.size() && curSelectedClips[idx])
                clip.traverse([this](const Clip& clip){
                    this->addRect(clip.min, clip.size, ImColor(1.f, 0.f, 0.f, 1.f));
                });
            else
                clip.traverse([this](const Clip& clip){
                    this->addRect(clip.min, clip.size, ImColor(0.f, 0.f, 1.f, 1.f));
                });
        }

        void addRect(glm::ivec2 pos, glm::ivec2 size, ImColor col)
        {
            _drawList->AddRect(
                {_wPos.x + _p0.x + scale(pos.x), _wPos.y + _p0.y + scale(pos.y)},
                {_wPos.x + _p0.x + scale(size.x + pos.x), _wPos.y + _p0.y + scale(size.y + pos.y)},
                col);
        }

        void addRectFilled(glm::ivec2 pos, glm::ivec2 size, ImColor col)
        {
            _drawList->AddRectFilled(
                {_wPos.x + _p0.x + scale(pos.x), _wPos.y + _p0.y + scale(pos.y)},
                {_wPos.x + _p0.x + scale(size.x + pos.x), _wPos.y + _p0.y + scale(size.y + pos.y)},
                col);
        }

        void addAlphaRect(glm::ivec2 pos, float size)
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
            glm::vec2 f = {};
            glm::ivec2 mi = {_imgs.front().pos.x, _imgs.front().pos.y};
            glm::ivec2 ma = {_imgs.front().pos.x, _imgs.front().pos.y};
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