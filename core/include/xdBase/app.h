#pragma once
#define IMGUI_ENABLE_FREETYPE

#ifndef NDEBUG
#include <iostream>
#endif

#include <array>
#include <list>
#include <locale>
#include <SDL.h>
#include <SDL_vulkan.h>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imguiMgr.h"
#include "staticEvent.h"
#include "staticEventMgr.h"
#include "timeMgr.h"
#include "vulkanMgr.h"

#define CC_MAX_LOOP_LAYER_IDX 0x1F

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

namespace XD::App {
    enum class EventSwitch : size_t {
        NoneType = SIZE_MAX,
        ResizeWindow = 0,
    };

    enum class LoopLayer : uint8_t {
        First = 0,
        Second = 1,
        Third = 2,
        WndBottom = 5,
        WndNormal = 6,
        WndTop = 7,
        Last = CC_MAX_LOOP_LAYER_IDX,
    };

    class LoopUnit {
    protected:
        explicit LoopUnit(LoopLayer layer = LoopLayer::First);
        virtual ~LoopUnit();
        virtual void start() = 0;
        virtual void update() = 0;

    private:
        friend void onUpdate(bool& quit, bool checkPlatformSpecialEvent);
        std::list<LoopUnit *>::const_iterator registerSelf();

        bool _started = false;
        const uint8_t _layer;
        const std::list<LoopUnit *>::const_iterator _selfApp;
    };

    /// @brief 空类
    class Default {};

    extern const std::locale loc;
}

namespace XD::App {
    class WndEventData
    {
    public:
        bool isWindowResizing   = false;
        bool isWindowMoving     = false;
    };

    // -----------------------------------------------------------------------------------------
    // 外调接口

    /// @brief 窗口宽
    /// @return 宽
    int getW();

    /// @brief 窗口高
    /// @return 高
    int getH();

    /// @brief 获取事件状态
    /// @param switchType 目标事件
    /// @return 事件状态 (未设置的事件状态为 UINT8_MAX)
    uint8_t getEventSwitch(const EventSwitch& switchType);

    /// @brief 获得窗口句柄
    /// @return 窗口句柄
    SDL_Window* getWHandle();

    /// @brief 获取程序开始时间点
    /// @return 程序开始时间点
    const clock_t& getStartTimePoint();

    /// @brief 设置事件状态
    /// @param switchType 目标事件
    /// @param state 要设置的状态
    void setEventSwitch(const EventSwitch& switchType, const uint8_t& state);

    /// @brief 设置背景颜色
    /// @param color 要设置的颜色
    void setBGColor(const ImVec4& color);

    /// @brief 退出程序
    void quit();

    /// @brief 各种事件状态
    /// @return 事件状态总类
    const WndEventData& event();

    // -----------------------------------------------------------------------------------------

    template<typename T>
    void logInfo(T x)
    {
#ifndef NDEBUG
        std::cout << x;
#endif
    }

    template<typename T, typename ...A>
    void logInfo(T x, A... args)
    {
#ifndef NDEBUG
        std::cout << x;
        logInfo(args...);
#endif
    }
}

#pragma clang diagnostic pop