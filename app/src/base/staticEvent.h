#pragma once

#include <filesystem>

#include "staticEventMgr.h"

namespace CC
{
    namespace StaticEvent
    {
        // ======================================= 鼠标事件
        /// @brief 鼠标点击事件
        class OnMouseClick : public EventTypeBase<OnMouseClick, uint8_t> {};
        /// @brief 鼠标长按事件
        class OnMouseLongPress : public EventTypeBase<OnMouseLongPress, uint8_t> {};
        /// @brief 鼠标按下
        class OnMouseDown : public EventTypeBase<OnMouseDown, uint8_t> {};
        /// @brief 鼠标抬起
        class OnMouseUp : public EventTypeBase<OnMouseUp, uint8_t> {};
        /// @brief 鼠标移动
        class OnMouseMove : public EventTypeBase<OnMouseMove> {};

        // ======================================= 文件事件
        /// @brief 文件拖拽
        class OnDropFile : public EventTypeBase<OnDropFile, std::filesystem::directory_entry> {};
        /// @brief 新文件入队
        class OnFilePush : public EventTypeBase<OnFilePush> {};
        /// @brief 新文件出队
        class OnFilePop : public EventTypeBase<OnFilePop> {};
    }
}