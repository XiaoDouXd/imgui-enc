#pragma once

#include <cstdint>

#include "vulkanMgr.h"

namespace XD::VulkanMgr {
    void init(const char** extensions, uint32_t extensionsCount);
    void destroy();
}