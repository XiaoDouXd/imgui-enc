#pragma once

#include "imguiMgr.h"

namespace XD::ImguiMgr {
    void init(SDL_Window* wSdl, VkSurfaceKHR& surf, int w, int h);
    void destroy();

    void presentFont();
    void prePresentFont();
    void present();
    void initDefaultStyle();
}
