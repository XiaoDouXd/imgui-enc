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

namespace XD
{
    enum class EventSwitch : size_t
    {
        NoneType = SIZE_MAX,
        ResizeWindow = 0,
    };

    enum class LoopLayer : uint8_t
    {
        First = 0,
        Sceond = 1,
        Third = 2,
        WndBottom = 5,
        WndNormal = 6,
        WndTop = 7,
        Last = CC_MAX_LOOP_LAYER_IDX,
    };

    class LoopUnit
    {
        friend class App;
    protected:
        LoopUnit(LoopLayer layer = LoopLayer::First);
        virtual ~LoopUnit();
        virtual void start() = 0;
        virtual void update() = 0;

    private:
        std::list<LoopUnit*>::const_iterator registerSelf();
        bool _started = false;
        const uint8_t _layer;
        const std::list<LoopUnit*>::const_iterator _selfApp;
    };

    class App
    {
    public:
        class WndEventData
        {
        public:
            bool isWindowResizing   = false;
            bool isWindowMoving     = false;
        };

    public:
        // -----------------------------------------------------------------------------------------
        // 外调接口

        /// @brief 窗口宽
        /// @return 宽
        static int getW();

        /// @brief 窗口高
        /// @return 高
        static int getH();

        /// @brief 获取事件状态
        /// @param switchType 目标事件
        /// @return 事件状态 (未设置的事件状态为 UINT8_MAX)
        static uint8_t getEventSwitch(const EventSwitch& switchType);

        /// @brief 获得窗口句柄
        /// @return 窗口句柄
        static SDL_Window* getWHandle();

        /// @brief 获取程序开始时间点
        /// @return 程序开始时间点
        static const clock_t& getStartTimePoint();

        /// @brief 设置事件状态
        /// @param switchType 目标事件
        /// @param state 要设置的状态
        static void setEventSwitch(const EventSwitch& switchType, const uint8_t& state);

        /// @brief 设置背景颜色
        /// @param color 要设置的颜色
        static void setBGColor(const ImVec4& color);

        /// @brief 退出程序
        static void quit();

        /// @brief 各种事件状态
        /// @return 事件状态总类
        static const WndEventData& event();

        // -----------------------------------------------------------------------------------------

        friend LoopUnit;
        friend class __app_caller;
    private:
        App() {}
        class WndData
        {
        public:
            SDL_Window*     window              = nullptr;
            VkSurfaceKHR    surface             = VK_NULL_HANDLE;

            std::vector<uint8_t> eventSwitch;
            clock_t         startClock;

            ImVec4          bgColor             = {255, 255, 255, 255};
            int             w                   = 0;
            int             h                   = 0;
            bool            willQuit            = false;
            bool            newLoopUnit         = false;
            bool            checkStart          = false;
            bool            firstFrame          = true;
        };

        static std::array<std::list<LoopUnit*>*, CC_MAX_LOOP_LAYER_IDX + 1> _loopUnits;
        static std::unique_ptr<WndData> _inst;
        static std::unique_ptr<WndEventData> _eventStateInst;

    private:
        // ---------------------------------- 内部函数

        static void init(const char* wndName);
        static void onUpdate(bool& quit, bool checkPlatformSpecialEvent = true);
        static void onDestroy();
        static void updateHead(bool& quit);
        static void updateTail();

        // ---------------------------------- 窗口事件检测
        static void eventRefresh(bool& quit, bool checkPlatformSpecialEvent = true);
        static void checkEvent(const SDL_Event& event);
        static void checkFrame();
        static void checkInit();

    public:
        template<typename T>
        static void logInfo(T x)
        {
#ifndef NDEBUG
            std::cout << x;
#endif
        }

        template<typename T, typename ...A>
        static void logInfo(T x, A... args)
        {
#ifndef NDEBUG
            std::cout << x;
            logInfo(args...);
#endif
        }
    };

    /// @brief 空类
    class Default {};

    extern const std::locale loc;
}