
#pragma once

#include "entrance.h"

namespace CC
{
    static inline bool rectTest(ImVec2 mousePos, ImVec2 rectMin, ImVec2 rectMax)
    {
        return  mousePos.x >= rectMin.x && mousePos.x <= rectMax.x &&
                mousePos.y >= rectMin.y && mousePos.y <= rectMax.y;
    }
}