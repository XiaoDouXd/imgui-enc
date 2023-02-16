#include "_/__app_caller.hpp"
#include "app.h"
#include "entrance.h"

#include "ui/unit/imgUnit.h"
#include "shortcut.h"

namespace CC
{
    enum class PosEdgeType : uint8_t { NotEdge = 0, LU = 5, LM = 4, LD = 8, MU = 1, MD = 3, RU = 6, RM = 2, RD = 7 };
    static PosEdgeType  mousePosType    = PosEdgeType::NotEdge;
    static PosEdgeType  mousePullFrom   = PosEdgeType::NotEdge;
    static bool         wMove           = false;
    static float mPosXprev, mPosYprev;
    static int wPosX, wPosY, wSizeW, wSizeH;
    static float mPosX, mPosY;
    static int wBtop, wBbottom, wBleft, wBright;

    static const clock_t mouseClickTimeLimit    = 1000;
    static bool         mouseDownStatePrev[5]   = {};
    static clock_t      mouseDownTime[5]        = {};

    // -----------------------------------------------------------------------------------------

    static inline PosEdgeType atEdge(const float& x, const float& y, const float& rw, const float& rh)
    {
        static const float deviation = 8.0f;
        static float dw, dh;
        dw = rw - deviation;
        dh = rh - deviation;

        if (x >= 0 && x < deviation) // 4 | 5 | 8
        {
            if (y >= 0 && y < deviation) return PosEdgeType::LU;
            if (y >= deviation && y <= dh) return PosEdgeType::LM;
            if (y > dh && y <= rh) return PosEdgeType::LD;
        }
        else if (x > dw && x <= rw) // 6 | 2 | 7
        {
            if (y >= 0 && y < deviation) return PosEdgeType::RU;
            if (y >= deviation && y <= dh) return PosEdgeType::RM;
            if (y > dh && y <= rh) return PosEdgeType::RD;
        }
        else if (x >= deviation && x <= dw) // 1 | 3
        {
            if (y >= 0 && y < deviation) return PosEdgeType::MU;
            if (y > dh && y <= rh) return PosEdgeType::MD;
        }
        return PosEdgeType::NotEdge;
    }

    static void checkMouseEvent(const uint8_t& mouseKey /*mouseKey: 0-左键 1-中键 2-右键*/)
    {
        // ------------------------------------------
        // 按下鼠标键
        if (ImguiMgr::getIO().MouseDown[mouseKey] && !mouseDownStatePrev[mouseKey])
        {
            mouseDownTime[mouseKey]        = TimeMgr::now();
            mouseDownStatePrev[mouseKey]   = true;
            StaticEventMgr::broadcastAsync<StaticEvent::OnMouseDown>(mouseKey);
        }
        if (!ImguiMgr::getIO().MouseDown[mouseKey] && mouseDownStatePrev[mouseKey])
        {
            mouseDownTime[mouseKey]        = TimeMgr::now() - mouseDownTime[mouseKey];
            mouseDownStatePrev[mouseKey]   = false;
            StaticEventMgr::broadcastAsync<StaticEvent::OnMouseUp>(mouseKey);
            if (mouseDownTime[mouseKey] < mouseClickTimeLimit)
                StaticEventMgr::broadcastAsync<StaticEvent::OnMouseClick>(mouseKey);
            else
                StaticEventMgr::broadcastAsync<StaticEvent::OnMouseLongPress>(mouseKey);
        }
    }

    // -----------------------------------------------------------------------------------------

    static void resizeWindowCursor()
    {
        SDL_GetWindowBordersSize(App::getWHandle(), &wBtop, &wBleft, &wBbottom, &wBright);
        if (App::getEventSwitch(EventSwitch::ResizeWindow))
        {
            static ImVec2 mPos;
            mPos = ImGui::GetMousePos();
            mousePosType = atEdge(mPos.x + wBleft, mPos.y + wBtop, App::getW() + wBright + wBleft, App::getH() + wBbottom + wBtop);
            switch (mousePosType)
            {
            case PosEdgeType::MD:
            case PosEdgeType::MU: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS); break;
            case PosEdgeType::LM:
            case PosEdgeType::RM: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW); break;
            case PosEdgeType::RU:
            case PosEdgeType::LD: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW); break;
            case PosEdgeType::LU:
            case PosEdgeType::RD: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE); break;
            }
        }
    }

    static void resizeWindow()
    {
#define CC_LIMIT_W_HEIGHT(x) std::max(x, (float)CC_WINDOW_LODING_HEIGHT)
#define CC_LIMIT_W_WIDTH(x)  std::max(x, (float)CC_WINDOW_LODING_WIDTH)
        if (!App::getEventSwitch(EventSwitch::ResizeWindow)) return;
        auto& io = ImguiMgr::getIO();
        static bool lock = false;

        if (mousePosType == PosEdgeType::NotEdge && mousePullFrom == PosEdgeType::NotEdge)
        {
            if (io.MouseDown[ImGuiMouseButton_Left]) lock = true;
            else lock = false;
        }
        if (lock) return;

        if (mousePosType == PosEdgeType::LU || mousePullFrom == PosEdgeType::LU)
        {
            if (io.MouseDown[ImGuiMouseButton_Left])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                if (mousePullFrom != PosEdgeType::LU)
                {
                    mousePullFrom = PosEdgeType::LU;
                    SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                        wSizeW - mPosX + mPosXprev <= CC_WINDOW_LODING_WIDTH ? wPosX : wPosX + mPosX - mPosXprev,
                        wSizeH - mPosY + mPosYprev <= CC_WINDOW_LODING_HEIGHT ? wPosY : wPosY + mPosY - mPosYprev);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW - mPosX + mPosXprev),
                        CC_LIMIT_W_HEIGHT(wSizeH - mPosY + mPosYprev));
                    mPosXprev = mPosX; mPosYprev = mPosY;

                    __app_caller::event().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::LU)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                __app_caller::event().isWindowResizing = false;
                StaticEventMgr::broadcast<StaticEvent::OnWindowResizeEnd>();
            }
        }
        else if (mousePosType == PosEdgeType::LD || mousePullFrom == PosEdgeType::LD)
        {
            if (io.MouseDown[ImGuiMouseButton_Left])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                if (mousePullFrom != PosEdgeType::LD)
                {
                    mousePullFrom = PosEdgeType::LD;
                    SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                        wSizeW - mPosX + mPosXprev <= CC_WINDOW_LODING_WIDTH ? wPosX : wPosX + mPosX - mPosXprev,
                        wPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW - mPosX + mPosXprev),
                        CC_LIMIT_W_HEIGHT(wSizeH + mPosY - mPosYprev));
                    mPosXprev = mPosX; mPosYprev = mPosY;

                    __app_caller::event().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::LD)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                __app_caller::event().isWindowResizing = false;
                StaticEventMgr::broadcast<StaticEvent::OnWindowResizeEnd>();
            }
        }
        else if (mousePosType == PosEdgeType::RU || mousePullFrom == PosEdgeType::RU)
        {
            if (io.MouseDown[ImGuiMouseButton_Left])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                if (mousePullFrom != PosEdgeType::RU)
                {
                    mousePullFrom = PosEdgeType::RU;
                    SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                        wPosX,
                        wSizeH - mPosY + mPosYprev <= CC_WINDOW_LODING_HEIGHT ? wPosY : wPosY + mPosY - mPosYprev);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW + mPosX - mPosXprev),
                        CC_LIMIT_W_HEIGHT(wSizeH - mPosY + mPosYprev));
                    mPosXprev = mPosX; mPosYprev = mPosY;

                    __app_caller::event().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::RU)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                __app_caller::event().isWindowResizing = false;
                StaticEventMgr::broadcast<StaticEvent::OnWindowResizeEnd>();
            }
        }
        else if (mousePosType == PosEdgeType::RD || mousePullFrom == PosEdgeType::RD)
        {
            if (io.MouseDown[ImGuiMouseButton_Left])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                if (mousePullFrom != PosEdgeType::RD)
                {
                    mousePullFrom = PosEdgeType::RD;
                    SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW + mPosX - mPosXprev),
                        CC_LIMIT_W_HEIGHT(wSizeH + mPosY - mPosYprev));
                    mPosXprev = mPosX; mPosYprev = mPosY;

                    __app_caller::event().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::RD)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                __app_caller::event().isWindowResizing = false;
                StaticEventMgr::broadcast<StaticEvent::OnWindowResizeEnd>();
            }
        }
        else if (mousePosType == PosEdgeType::LM || mousePullFrom == PosEdgeType::LM)
        {
            if (io.MouseDown[ImGuiMouseButton_Left])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                if (mousePullFrom != PosEdgeType::LM)
                {
                    mousePullFrom = PosEdgeType::LM;
                    SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                        wSizeW - mPosX + mPosXprev <= CC_WINDOW_LODING_WIDTH ? wPosX : wPosX + mPosX - mPosXprev,
                        wPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW - mPosX + mPosXprev),
                        wSizeH);
                    mPosXprev = mPosX; mPosYprev = mPosY;

                    __app_caller::event().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::LM)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                __app_caller::event().isWindowResizing = false;
                StaticEventMgr::broadcast<StaticEvent::OnWindowResizeEnd>();
            }
        }
        else if (mousePosType == PosEdgeType::MU || mousePullFrom == PosEdgeType::MU)
        {
            if (io.MouseDown[ImGuiMouseButton_Left])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                if (mousePullFrom != PosEdgeType::MU)
                {
                    mousePullFrom = PosEdgeType::MU;
                    SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                        wPosX,
                        wSizeH - mPosY + mPosYprev <= CC_WINDOW_LODING_HEIGHT ? wPosY : wPosY + mPosY - mPosYprev);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        wSizeW,
                        CC_LIMIT_W_HEIGHT(wSizeH - mPosY + mPosYprev));
                    mPosXprev = mPosX; mPosYprev = mPosY;

                    __app_caller::event().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::MU)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                __app_caller::event().isWindowResizing = false;
                StaticEventMgr::broadcast<StaticEvent::OnWindowResizeEnd>();
            }
        }
        else if (mousePosType == PosEdgeType::RM || mousePullFrom == PosEdgeType::RM)
        {
            if (io.MouseDown[ImGuiMouseButton_Left])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                if (mousePullFrom != PosEdgeType::RM)
                {
                    mousePullFrom = PosEdgeType::RM;
                    SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW + mPosX - mPosXprev),
                        wSizeH);
                    mPosXprev = mPosX; mPosYprev = mPosY;

                    __app_caller::event().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::RM)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                __app_caller::event().isWindowResizing = false;
                StaticEventMgr::broadcast<StaticEvent::OnWindowResizeEnd>();
            }
        }
        else if (mousePosType == PosEdgeType::MD || mousePullFrom == PosEdgeType::MD)
        {
            if (io.MouseDown[ImGuiMouseButton_Left])
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                if (mousePullFrom != PosEdgeType::MD)
                {
                    mousePullFrom = PosEdgeType::MD;
                    SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        wSizeW,
                        CC_LIMIT_W_HEIGHT(wSizeH + mPosY - mPosYprev));
                    mPosXprev = mPosX; mPosYprev = mPosY;

                    __app_caller::event().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::MD)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                __app_caller::event().isWindowResizing = false;
                StaticEventMgr::broadcast<StaticEvent::OnWindowResizeEnd>();
            }
        }

#undef CC_LIMIT_W_HEIGHT
#undef CC_LIMIT_W_WIDTH
    }

    static void moveWindow()
    {
        auto& io = ImguiMgr::getIO();
        if (io.MouseDown[ImGuiMouseButton_Middle] || io.MouseDown[ImGuiMouseButton_Right])
        {
            static float mPosXprev, mPosYprev;
            static int wPosX, wPosY;
            static float mPosX, mPosY;
            if (!wMove)
            {
                SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
                __app_caller::event().isWindowMoving = wMove = true;
            }
            else
            {
                SDL_GetGlobalMouseState(&mPosX, &mPosY);
                SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                SDL_SetWindowPosition(App::getWHandle(), wPosX + mPosX - mPosXprev, wPosY + mPosY - mPosYprev);
                mPosXprev = mPosX; mPosYprev = mPosY;
            }
        }
        else __app_caller::event().isWindowMoving = wMove = false;
    }

    static void mouseCheck()
    {
        checkMouseEvent(0);
        checkMouseEvent(1);
        checkMouseEvent(2);

        SDL_GetGlobalMouseState(&mPosX, &mPosY);
        if (abs(mPosX - mPosXprev) >= 0.1f || abs(mPosY - mPosYprev) >= 0.1f)
            StaticEventMgr::broadcastAsync<StaticEvent::OnMouseMove>();
    }

    static void dropFile(const SDL_Event& event)
    {
        if (event.type == SDL_DROPFILE)
        {
            if (!event.drop.file || !*event.drop.file) return;

            using namespace std::filesystem;
            auto oldLoc = std::locale::global(UI::loc);
            auto dir = directory_entry(std::u8string((char8_t*)event.drop.file));
            std::locale::global(oldLoc);
            StaticEventMgr::broadcastAsync<StaticEvent::OnDropFile>(dir);
        }
    }

    static void shortcut(const SDL_Event& event)
    {
        if (event.type == SDL_KEYDOWN && !event.key.repeat)
        {
            for (size_t i = 0; i < ShortcutMap.size(); i++)
            {
                if (!isHit(ShortcutMap[i],
                    (SDL_KeyCode)event.key.keysym.sym,
                    (SDL_Keymod)event.key.keysym.mod)) continue;
                StaticEventMgr::broadcastAsync<StaticEvent::OnShortcut>((Shortcut)i);
            }
        }
    }

    // -----------------------------------------------------------------------------------------

    /// @brief 事件处理数据初始化
    void App::checkInit()
    {
        SDL_GetGlobalMouseState(&mPosXprev, &mPosYprev);
    }

    /// @brief 每帧检测的事件处理核心
    void App::checkFrame()
    {
        // resizeWindowCursor();
        mouseCheck();
    }

    /// @brief 事件处理核心
    /// @param event 事件在处理
    void App::checkEvent(const SDL_Event& event)
    {
        // resizeWindow();
        moveWindow();
        dropFile(event);
        shortcut(event);
    }
}