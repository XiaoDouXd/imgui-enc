#include "entrance.h"
#include "p_app.h"

namespace XD
{
    const std::locale loc = std::locale();

    enum class PosEdgeType : uint8_t { NotEdge = 0, LU = 5, LM = 4, LD = 8, MU = 1, MD = 3, RU = 6, RM = 2, RD = 7 };
    static PosEdgeType  mousePosType    = PosEdgeType::NotEdge;
    static PosEdgeType  mousePullFrom   = PosEdgeType::NotEdge;
    static bool         wMove           = false;
    static float mPosXPrev, mPosYPrev;
    static int wPosX, wPosY, wSizeW, wSizeH;
    static float mPosX, mPosY;
    static int wBTop, wBBottom, wBLeft, wBRight;

    static const clock_t mouseClickTimeLimit    = 1000;
    static bool         mouseDownStatePrev[5]   = {};
    static clock_t      mouseDownTime[5]        = {};

    // -----------------------------------------------------------------------------------------

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCallsOfFunction"
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
#pragma clang diagnostic pop

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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    static void resizeWindowCursor()
    {
        SDL_GetWindowBordersSize(App::getWHandle(), &wBTop, &wBLeft, &wBBottom, &wBRight);
        if (App::getEventSwitch(App::EventSwitch::ResizeWindow))
        {
            static ImVec2 mPos;
            mPos = ImGui::GetMousePos();
            mousePosType = atEdge(mPos.x + (float)wBLeft, mPos.y + (float)wBTop,
                                  (float)App::getW() + (float)wBRight + (float)wBLeft,
                                  (float)App::getH() + (float)wBBottom + (float)wBTop);
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
                case PosEdgeType::NotEdge:
                    break;
            }
        }
    }
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    static void resizeWindow()
    {
#define CC_LIMIT_W_HEIGHT(x) std::max(x, (float)xdWndInitConf_loadingHeight)
#define CC_LIMIT_W_WIDTH(x)  std::max(x, (float)xdWndInitConf_loadingWidth)
        if (!App::getEventSwitch(App::EventSwitch::ResizeWindow)) return;
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
                    SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                                          (float)wSizeW - mPosX + mPosXPrev <= (float)xdWndInitConf_loadingWidth
                                          ? wPosX : wPosX + (int)mPosX - (int)mPosXPrev,
                                          (float)wSizeH - mPosY + mPosYPrev <= (float)xdWndInitConf_loadingHeight
                                          ? wPosY : wPosY + (int)mPosY - (int)mPosYPrev);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW - mPosX + mPosXPrev),
                        CC_LIMIT_W_HEIGHT(wSizeH - mPosY + mPosYPrev));
                    mPosXPrev = mPosX; mPosYPrev = mPosY;

                    App::eventData().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::LU)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                App::eventData().isWindowResizing = false;
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
                    SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                                          (float)wSizeW - mPosX + mPosXPrev <= (float)xdWndInitConf_loadingWidth
                                          ? wPosX : wPosX + (int)mPosX - (int)mPosXPrev,
                        wPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW - mPosX + mPosXPrev),
                        CC_LIMIT_W_HEIGHT(wSizeH + mPosY - mPosYPrev));
                    mPosXPrev = mPosX; mPosYPrev = mPosY;

                    App::eventData().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::LD)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                App::eventData().isWindowResizing = false;
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
                    SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                        wPosX,
                        (float)wSizeH - mPosY + mPosYPrev <= (float)xdWndInitConf_loadingHeight
                        ? wPosY : wPosY + (int)mPosY - (int)mPosYPrev);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW + mPosX - mPosXPrev),
                        CC_LIMIT_W_HEIGHT(wSizeH - mPosY + mPosYPrev));
                    mPosXPrev = mPosX; mPosYPrev = mPosY;

                    App::eventData().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::RU)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                App::eventData().isWindowResizing = false;
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
                    SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW + mPosX - mPosXPrev),
                        CC_LIMIT_W_HEIGHT(wSizeH + mPosY - mPosYPrev));
                    mPosXPrev = mPosX; mPosYPrev = mPosY;

                    App::eventData().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::RD)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                App::eventData().isWindowResizing = false;
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
                    SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                                          (float)wSizeW - mPosX + mPosXPrev <= (float)xdWndInitConf_loadingWidth
                                          ? wPosX : wPosX + (int)mPosX - (int)mPosXPrev,
                        wPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW - mPosX + mPosXPrev),
                        wSizeH);
                    mPosXPrev = mPosX; mPosYPrev = mPosY;

                    App::eventData().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::LM)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                App::eventData().isWindowResizing = false;
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
                    SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                    SDL_SetWindowPosition(App::getWHandle(),
                        wPosX,
                        (float)wSizeH - mPosY + mPosYPrev <= (float)xdWndInitConf_loadingHeight
                        ? wPosY : wPosY + (int)mPosY - (int)mPosYPrev);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        wSizeW,
                        CC_LIMIT_W_HEIGHT(wSizeH - mPosY + mPosYPrev));
                    mPosXPrev = mPosX; mPosYPrev = mPosY;

                    App::eventData().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::MU)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                App::eventData().isWindowResizing = false;
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
                    SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        CC_LIMIT_W_WIDTH(wSizeW + mPosX - mPosXPrev),
                        wSizeH);
                    mPosXPrev = mPosX; mPosYPrev = mPosY;

                    App::eventData().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::RM)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                App::eventData().isWindowResizing = false;
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
                    SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
                }
                else
                {
                    SDL_GetGlobalMouseState(&mPosX, &mPosY);
                    SDL_GetWindowSize(App::getWHandle(), &wSizeW, &wSizeH);
                    SDL_SetWindowSize(App::getWHandle(),
                        wSizeW,
                        CC_LIMIT_W_HEIGHT(wSizeH + mPosY - mPosYPrev));
                    mPosXPrev = mPosX; mPosYPrev = mPosY;

                    App::eventData().isWindowResizing = true;
                    StaticEventMgr::broadcast<StaticEvent::OnWindowResizeBegin>();
                }
            }
            else if (mousePullFrom == PosEdgeType::MD)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                mousePullFrom = PosEdgeType::NotEdge;
                App::eventData().isWindowResizing = false;
                StaticEventMgr::broadcast<StaticEvent::OnWindowResizeEnd>();
            }
        }

#undef CC_LIMIT_W_HEIGHT
#undef CC_LIMIT_W_WIDTH
    }
#pragma clang diagnostic pop

    static void moveWindow()
    {
        auto& io = ImguiMgr::getIO();
        if (io.MouseDown[ImGuiMouseButton_Middle] || io.MouseDown[ImGuiMouseButton_Right])
        {
            static float mPosXPrev, mPosYPrev;
            static int wPosX, wPosY;
            static float mPosX, mPosY;
            if (!wMove)
            {
                SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
                App::eventData().isWindowMoving = wMove = true;
            }
            else
            {
                SDL_GetGlobalMouseState(&mPosX, &mPosY);
                SDL_GetWindowPosition(App::getWHandle(), &wPosX, &wPosY);
                SDL_SetWindowPosition(App::getWHandle(),
                                      wPosX + (int)mPosX - (int)mPosXPrev, wPosY + (int)mPosY - (int)mPosYPrev);
                mPosXPrev = mPosX; mPosYPrev = mPosY;
            }
        }
        else App::eventData().isWindowMoving = wMove = false;
    }

    static void mouseCheck()
    {
        checkMouseEvent(0);
        checkMouseEvent(1);
        checkMouseEvent(2);

        SDL_GetGlobalMouseState(&mPosX, &mPosY);
        if (abs(mPosX - mPosXPrev) >= 0.1f || abs(mPosY - mPosYPrev) >= 0.1f)
            StaticEventMgr::broadcastAsync<StaticEvent::OnMouseMove>();
    }

    static void dropFile(const SDL_Event& event)
    {
        if (event.type == SDL_DROPFILE)
        {
            if (!event.drop.file || !*event.drop.file) return;

            using namespace std::filesystem;
            auto oldLoc = std::locale::global(loc);
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
        SDL_GetGlobalMouseState(&mPosXPrev, &mPosYPrev);
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