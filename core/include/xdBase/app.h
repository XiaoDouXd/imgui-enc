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

    /// @brief loopLayer def
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
        /// @brief the basic unit of loop
        /// @param layer updated first of others layer number is bigger
        explicit LoopUnit(LoopLayer layer = LoopLayer::First);
        virtual ~LoopUnit();

        /// @brief called while instance created first
        virtual void start() = 0;

        /// @brief called every frame
        virtual void update() = 0;

    private:
        friend void onUpdate(bool& quit, bool checkPlatformSpecialEvent);
        std::list<LoopUnit *>::const_iterator registerSelf();

        bool _started = false;
        const uint8_t _layer;
        const std::list<LoopUnit *>::const_iterator _selfApp;
    };

    /// @brief empty class
    class Default {};

    /// @brief loc env
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
    // interface

    /// @brief window width
    int getW();

    /// @brief window height
    int getH();

    /// @brief get event states
    /// @param switchType target event
    /// @return event state (return UINT8_MAX while event is not set)
    uint8_t getEventSwitch(const EventSwitch& switchType);

    /// @brief get sdl wnd handle
    SDL_Window* getWHandle();

    /// @brief get program start time
    const clock_t& getStartTimePoint();

    /// @brief set event states
    /// @param switchType target event
    /// @param state state to set
    void setEventSwitch(const EventSwitch& switchType, const uint8_t& state);

    /// @brief set bg color
    /// @param color color
    void setBGColor(const ImVec4& color);

    /// @brief quit
    void quit();

    /// @brief get all events
    /// @return the event states data class of
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