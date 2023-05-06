#pragma once

#include <cstdint>

namespace XD::WndMgr {
    void init();
    void destroy();
    void wndGC();
    void update();
    size_t newId();
}