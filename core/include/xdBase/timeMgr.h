#pragma once

#include <chrono>
#include <ctime>
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <thread>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

namespace XD::TimeMgr {
    /// @brief now time
    clock_t now();

    /// @brief now time
    std::chrono::high_resolution_clock::time_point nowTimePoint();

    /// @brief refresh time, called by appMgr
    void refresh();

    /// @brief delay callback
    /// @param cb callback
    /// @param delay delay (ms)
    void delay(const std::function<void()>& cb, clock_t delay);
}
#pragma clang diagnostic pop