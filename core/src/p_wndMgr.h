#pragma once

#include <cstdint>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

namespace XD::WndMgr {
    void init();
    void destroy();
    void wndGC();
    void update();
    size_t newId();
}
#pragma clang diagnostic pop