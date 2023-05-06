#pragma once

#include <filesystem>

#include "shortcut.h"
#include "staticEventMgr.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedStructInspection"

namespace XD::StaticEvent {
    // ======================================= window events
    /// @brief begin resize window
    class OnWindowResizeBegin : public EventTypeBase<OnWindowResizeBegin> {};
    /// @brief end resize window
    class OnWindowResizeEnd : public EventTypeBase<OnWindowResizeEnd> {};

    // ======================================= mouse events
    /// @brief click
    class OnMouseClick : public EventTypeBase<OnMouseClick, uint8_t> {};
    /// @brief long press
    class OnMouseLongPress : public EventTypeBase<OnMouseLongPress, uint8_t> {};
    /// @brief down
    class OnMouseDown : public EventTypeBase<OnMouseDown, uint8_t> {};
    /// @brief up
    class OnMouseUp : public EventTypeBase<OnMouseUp, uint8_t> {};
    /// @brief moving
    class OnMouseMove : public EventTypeBase<OnMouseMove> {};

    // ======================================= keyboard events
    /// @brief down
    class OnKeyDown : public EventTypeBase<OnKeyDown, int> {};
    /// @brief shortcut
    class OnShortcut : public EventTypeBase<OnShortcut, Shortcut> {};

    // ======================================= file events
    /// @brief dropping file
    class OnDropFile : public EventTypeBase<OnDropFile, std::filesystem::directory_entry> {};
    /// @brief enqueue file
    class OnFilePush : public EventTypeBase<OnFilePush> {};
    /// @brief dequeue file
    class OnFilePop : public EventTypeBase<OnFilePop> {};
}
#pragma clang diagnostic pop