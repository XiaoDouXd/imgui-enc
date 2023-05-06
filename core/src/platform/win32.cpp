#ifdef WIN32
#include "univesal.h"

#include <SDL.h>
#include <Windows.h>
#include <SDL_syswm.h>

#include "p_app.h"
#include "app.h"

namespace XD
{
    #define SIZE_MOVE_TIMER_ID 1
    static bool sizeMoveTimerRunning = false;
    static auto quit = false;

    static int eventWatch(void*, SDL_Event* event) {
        if (event->type == SDL_SYSWMEVENT) {
            const auto& winMessage = event->syswm.msg->msg.win;
            if (winMessage.msg == WM_ENTERSIZEMOVE) {
                sizeMoveTimerRunning = SetTimer(GetActiveWindow(), SIZE_MOVE_TIMER_ID, USER_TIMER_MINIMUM, nullptr);
            }
            else if (winMessage.msg == WM_TIMER) {
                if (winMessage.wParam == SIZE_MOVE_TIMER_ID) {
                    App::onUpdate(quit, false);
                    if (quit) App::quit();
                }
            }
        }
        return 0;
    }

    void platformSpecialInit() {
        SDL_AddEventWatch(eventWatch, nullptr);
        SDL_SetEventEnabled(SDL_SYSWMEVENT, SDL_TRUE);
    }

    void platformSpecialEvent([[maybe_unused]] bool& unused, [[maybe_unused]] const SDL_Event& unused2)
    {
        if (sizeMoveTimerRunning) {
            KillTimer(GetActiveWindow(), SIZE_MOVE_TIMER_ID);
            sizeMoveTimerRunning = false;
        }
    }
}
#endif