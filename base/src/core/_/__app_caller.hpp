#include "app.h"

namespace XD
{
    class __app_caller
    {
    private: __app_caller() {}
    public:
        static void init(const char *wndName) { App::init(wndName); }
        static void onUpdate(bool& quit, bool checkPlatformSpecialEvent = true) { App::onUpdate(quit, checkPlatformSpecialEvent); }
        static void onDestroy() { App::onDestroy(); }
        static void eventRefresh(bool& quit) { App::eventRefresh(quit); }

        static App::WndEventData& event() { return *App::_eventStateInst; }
    };
}