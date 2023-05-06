#pragma once

#include <chrono>
#include <ctime>
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <thread>

namespace XD::TimeMgr
{
        clock_t now();
        std::chrono::high_resolution_clock::time_point nowTimePoint();
        void refresh();

        /// @brief 延迟回调
        /// @param cb 回调
        /// @param delay 延迟(ms)
        void delay(const std::function<void()>& cb, clock_t delay);
}