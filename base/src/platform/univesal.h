#pragma once

#include <SDL.h>

#ifdef WIN32

#else   // 不需要对平台做特殊处理时
#define CC_NO_PLATFORM_SPECIAL_INIT
#define CC_NO_PLATFORM_SPECIAL_EVENT
#endif

namespace XD
{
    /// @brief 用于特定平台的初始化 调用在 app.cpp/main() 中, init() 函数前
    void platformSpecialInit();

    /// @brief  用于特定平台的系统事件刷新 调用在 app.cpp/App::eventRefresh() 中
    ///         具体运行与否受到 App::onUpdate() 的 checkPlatformSpecialEvent 参数控制
    /// @param quit 是否关闭窗口
    /// @param event 传入的 SDL 事件
    void platformSpecialEvent(bool& quit, const SDL_Event& event);
}