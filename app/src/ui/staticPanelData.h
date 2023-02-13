#pragma once

#include <cstdint>
#include <vector>

#include "core/clipCtrl.h"
#include "glm.hpp"

namespace CC::UI
{
    extern ssize_t curDragedClip;
    extern ssize_t curHoveredClip;
    extern std::vector<uint8_t> curSelectedClips;
    extern std::vector<glm::vec4> clipChanges;
    extern bool curSelectedClipsReset;
} // namespace CC
