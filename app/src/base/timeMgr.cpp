#include "timeMgr.h"

namespace CC
{
    std::unique_ptr<TimeMgr::TimeMgrData> TimeMgr::_inst = nullptr;

    void TimeMgr::init()
    {
        if (_inst) return;
        _inst = std::make_unique<TimeMgrData>();
    }

    void TimeMgr::destory()
    {
        if (_inst) _inst.reset();
    }

    clock_t TimeMgr::now()
    {
        return clock();
    }

    std::chrono::high_resolution_clock::time_point TimeMgr::nowTimePoint()
    {
        return std::chrono::high_resolution_clock::now();
    }

    void TimeMgr::refresh()
    {
        if (_inst->callbacks.empty()) return;
        static bool run;
        do
        {
            run = now() >= _inst->callbacks.top().endTime;
            if (run)
            {
                _inst->callbacks.top().cb();
                _inst->callbacks.pop();
            }
        } while (run);
    }

    void TimeMgr::delay(std::function<void()> cb, clock_t delay)
    {
        if (_inst->callbacks.empty()) return;
        _inst->callbacks.push(TimeCallback(cb, delay + now()));
    }
}