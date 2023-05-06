#pragma once

#include "app.h"
#include "SDL_events.h"

namespace XD::App {
    // ---------------------------------- 内部函数
    void init(const char* wndName); // NOLINT(readability-redundant-declaration)
    void onUpdate(bool& quit, bool checkPlatformSpecialEvent = true); // NOLINT(readability-redundant-declaration)
    void onDestroy(); // NOLINT(readability-redundant-declaration)
    void updateHead(bool& quit); // NOLINT(readability-redundant-declaration)
    void updateTail(); // NOLINT(readability-redundant-declaration)

    // ---------------------------------- 窗口事件检测
    void eventRefresh(bool& quit, bool checkPlatformSpecialEvent = true);
    void checkEvent(const SDL_Event& event);
    void checkFrame();
    void checkInit();
    WndEventData& eventData();
}