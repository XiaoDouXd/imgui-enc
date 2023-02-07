#pragma once

#include <chrono>
#include <ctime>
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <thread>

namespace CC
{
    class TimeMgr
    {
        friend class App;
        friend class TimeUnit;
    private:
        TimeMgr() {}
        class TimeCallback
        {
        public:
            clock_t                 endTime;
            std::function<void()>   cb;
            TimeCallback(std::function<void()> cb, double endTime) :
            endTime(endTime), cb(cb) {}
        };
        class TimeCallbackCompare
        {
        public:
            bool operator()(const TimeCallback& a, const TimeCallback& b)
            { return a.endTime > b.endTime; }
        };
        class TimeMgrData
        {
        public:
            std::chrono::high_resolution_clock::time_point startPoint;
            std::priority_queue<
                TimeCallback,
                std::vector<TimeCallback>,
                TimeCallbackCompare> callbacks;
        };
        static std::unique_ptr<TimeMgrData> _inst;

        static void init();
        static void destory();
    public:
        static clock_t now();
        static std::chrono::high_resolution_clock::time_point nowTimePoint();
        static void refresh();

        /// @brief 延迟回调
        /// @param cb 回调
        /// @param delay 延迟(ms)
        static void delay(std::function<void()> cb, clock_t delay);
    };
}