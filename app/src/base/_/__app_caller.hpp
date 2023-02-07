#include "app.h"

namespace CC
{
    class __app_caller
    {
    private: __app_caller() {}
    public:
        static void init(const char *wndName) { App::init(wndName); }
        static void onUpdate(bool& quit) { App::onUpdate(quit); }
        static void onDestroy() { App::onDestroy(); }

        static App::WndEventData& event() { return *App::_eventStateInst; }
    };
}