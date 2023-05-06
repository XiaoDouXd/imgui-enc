#include <iostream>
#include <SDL3/SDL_image.h>

#include "entrance.h"
#include "imgui_impl_sdl.h"
#include "platform/univesal.h"

#include "wndMgr.h"
#include "p_app.h"
#include "p_imguiMgr.h"
#include "p_timeMgr.h"
#include "p_vulkanMgr.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
namespace XD::App
{
    class WndData
    {
    public:
        SDL_Window*     window              = nullptr;
        VkSurfaceKHR    surface             = VK_NULL_HANDLE;

        std::vector<uint8_t> eventSwitch;
        clock_t         startClock{};

        ImVec4          bgColor             = {255, 255, 255, 255};
        int             w                   = 0;
        int             h                   = 0;
        bool            willQuit            = false;
        bool            newLoopUnit         = false;
        bool            checkStart          = false;
        bool            firstFrame          = true;
    };

    std::array<std::list<LoopUnit*>*, CC_MAX_LOOP_LAYER_IDX + 1> _loopUnits;
    std::unique_ptr<WndData> _inst;
    std::unique_ptr<WndEventData> _eventStateInst;

    WndEventData& eventData() {return *_eventStateInst;}

    uint8_t getEventSwitch(const EventSwitch& switchType)
    {
        static size_t typeIdx; typeIdx = (size_t)switchType;
        if (_inst->eventSwitch.size() <= typeIdx) return UINT8_MAX;
        return _inst->eventSwitch[typeIdx];
    }

    void setEventSwitch(const EventSwitch& switchType, const uint8_t& state)
    {
        static size_t typeIdx; typeIdx = (size_t)switchType;
        if (_inst->eventSwitch.size() <= typeIdx)
            _inst->eventSwitch.resize(typeIdx + 1, UINT8_MAX);
        _inst->eventSwitch[typeIdx] = state;
    }

    int getH() { return _inst->h; }

    int getW() { return _inst->w; }

    const clock_t& getStartTimePoint()
    {
        return _inst->startClock;
    }

    SDL_Window* getWHandle() { return _inst->window; }

    void setBGColor(const ImVec4& color) { _inst->bgColor = color; }

    void quit() { _inst->willQuit = true; }

    const WndEventData& event() { return *_eventStateInst; }
}

namespace XD::App
{
    static inline bool isWindowEvent(const SDL_Event& event)
    {
        return event.type >= SDL_EventType::SDL_WINDOWEVENT_FIRST && event.type <= SDL_EventType::SDL_WINDOWEVENT_LAST;
    }

    static inline bool isEvent(const SDL_Event& event, const SDL_EventType& from, const SDL_EventType& to)
    {
        return event.type >= from && event.type <= to;
    }
    static bool wndBoarded = false;

    // -----------------------------------------------------------------------------------------

    void init(const char* wndName)
    {
        if (!_inst) _inst = std::make_unique<WndData>();
        if (!_eventStateInst) _eventStateInst = std::make_unique<WndEventData>();

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0)
            throw Exce(__LINE__, __FILE__, "XD::App: SDL 初始化失败");
        if (SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1") != SDL_TRUE)
            throw Exce(__LINE__, __FILE__, "XD::App: SDL 输入法设置失败");
        TimeMgr::init();
        StaticEventMgr::init();

        _inst->startClock = TimeMgr::now();

        // 初始化 imgui 上下文
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // 绑定到 vulkan 实例
        auto wndFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
        wndBoarded = !(wndFlags & SDL_WINDOW_BORDERLESS);

        if (xdWndInitConf_waitTime >= 0.5)
        {
            _inst->window = SDL_CreateWindow(wndName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, xdWndInitConf_loadingWidth, xdWndInitConf_loadingHeight, wndFlags);
            SDL_SetWindowBordered(_inst->window, SDL_FALSE);
            setEventSwitch(EventSwitch::ResizeWindow, 0);
        }
        else
        {
            _inst->window = SDL_CreateWindow(wndName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, xdWndInitConf_defWndWidth - 1, xdWndInitConf_defWndHeight, wndFlags);
            SDL_SetWindowMinimumSize(_inst->window, xdWndInitConf_loadingWidth, xdWndInitConf_loadingHeight);
        }
        int w, h;
        SDL_GetWindowSize(_inst->window, &w, &h);

        if (xdWndInitConf_iconPngData && xdWndInitConf_iconPngSize)
        {
            auto iconSurf = SDL_RWFromConstMem((void*)xdWndInitConf_iconPngData, (int)xdWndInitConf_iconPngSize);
            auto icon = IMG_LoadPNG_RW(iconSurf);
            SDL_SetWindowIcon(_inst->window, icon);
        }
        uint32_t extensionsCount = 0;
        SDL_Vulkan_GetInstanceExtensions(_inst->window, &extensionsCount, nullptr);
        const char** extensions = new const char*[extensionsCount];
        SDL_Vulkan_GetInstanceExtensions(_inst->window, &extensionsCount, extensions);
        VulkanMgr::init(extensions, extensionsCount);

        if (SDL_Vulkan_CreateSurface(_inst->window, VulkanMgr::getInst(), &(_inst->surface)) == 0)
            throw Exce(__LINE__, __FILE__, "XD::App: SDL Surface 创建失败");
        _inst->w = w; _inst->h = h;
        ImguiMgr::init(_inst->window, _inst->surface, _inst->w, _inst->h);
        WndMgr::init();

        if (xdWndInitConf_waitTime < 0.5)
        {
            setEventSwitch(EventSwitch::ResizeWindow, UINT8_MAX);
            ImguiMgr::disableLoading();
        }
        delete[] extensions;

        // 初始化窗口检测事件
        checkInit();
    }

    void eventRefresh(bool& quit, bool checkPlatformSpecialEvent)
    {
        // 在此处处理事件处理事件
        // 设置 io.WantCaptureMouse, io.WantCaptureKeyboard 让 imgui 使用自定义输入
        // - io.WantCaptureMouse 为 true 时, 不再处理鼠标输入事件
        // - io.WantCaptureKeyboard 为 true 时, 不再捕获键盘输入事件
        static SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // 平台特化的事件处理
#ifndef CC_NO_PLATFORM_SPECIAL_EVENT
            if(checkPlatformSpecialEvent) platformSpecialEvent(quit, event);
#endif
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                quit = true;
            if (isWindowEvent(event) && event.window.type == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(_inst->window))
                quit = true;
            checkEvent(event);
        }
    }

    void updateHead(bool& quit [[maybe_unused]])
    {
        checkFrame();

        // 重设交换链大小
        ImguiMgr::preRender(_inst->window, _inst->w, _inst->h);
        TimeMgr::refresh();
    }

    void updateTail()
    {
        WndMgr::update();
        ImguiMgr::render(_inst->bgColor);
        SDL_GetWindowSize(_inst->window, &(_inst->w), &(_inst->h));

        if (xdWndInitConf_waitTime >= 0.5)
        {
            if (ImguiMgr::notLoadingFinished() && !_inst->firstFrame &&
                (TimeMgr::now() - _inst->startClock) > (xdWndInitConf_waitTime * 1000))
            {
                static int x, y;
                if (wndBoarded) SDL_SetWindowBordered(_inst->window, SDL_TRUE);
                SDL_SetWindowSize(_inst->window, xdWndInitConf_defWndWidth, xdWndInitConf_defWndHeight);
                SDL_GetWindowPosition(_inst->window, &x, &y);
                SDL_SetWindowPosition(_inst->window,
                    x - ((int)(xdWndInitConf_defWndWidth - xdWndInitConf_loadingWidth) >> 1),
                    y - ((int)(xdWndInitConf_defWndHeight - xdWndInitConf_loadingHeight) >> 1));
                setEventSwitch(EventSwitch::ResizeWindow, UINT8_MAX);
                ImguiMgr::finishLoading();
            }
        }

        if (_inst->firstFrame)
        {
            if (xdWndInitConf_waitTime >= 0.5)
                SDL_SetWindowSize(_inst->window, xdWndInitConf_loadingWidth, xdWndInitConf_loadingHeight);
            else
                SDL_SetWindowSize(_inst->window, xdWndInitConf_defWndWidth, xdWndInitConf_defWndHeight);
            _inst->firstFrame = false;
        }
    }

    void onUpdate(bool& quit, bool checkPlatformSpecialEvent)
    {
        eventRefresh(quit, checkPlatformSpecialEvent);
        updateHead(quit);
        quit |= _inst->willQuit;
        _inst->checkStart = _inst->newLoopUnit;
        _inst->newLoopUnit = false;
        for (auto loopLayer : _loopUnits)
        {
            if (!loopLayer) continue;
            if (_inst->checkStart)
            {
                for (auto loopUnit : *loopLayer)
                {
                    if (loopUnit->_started) continue;
                    loopUnit->start();
                    loopUnit->_started = true;
                }
            }
            for (auto loopUnit : *loopLayer)
                loopUnit->update();
        }
        _inst->checkStart = false;
        TimeMgr::refresh();
        StaticEventMgr::update();
        updateTail();
    }

    void onDestroy()
    {
        auto err = vkDeviceWaitIdle(XD::VulkanMgr::getDev());
        XD::VulkanMgr::checkVkResultCType(err);

        WndMgr::destroy();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        ImguiMgr::destroy();
        VulkanMgr::destroy();
        TimeMgr::destroy();

        SDL_DestroyWindow(_inst->window);
        SDL_Quit();

        _inst.reset();
    }

    // -----------------------------------------------------------------------------------------

    LoopUnit::LoopUnit(LoopLayer layer) : _layer((uint8_t)layer), _selfApp(registerSelf())
    {
        if (!App::_inst) throw Exce(__LINE__, __FILE__, "XD::LoopUnit: App 未初始化");
        App::_inst->newLoopUnit = true;
    }

    std::list<LoopUnit*>::const_iterator LoopUnit::registerSelf()
    {
        if (_layer < 0 || _layer > CC_MAX_LOOP_LAYER_IDX)
            throw Exce(__LINE__, __FILE__, "XD::LoopUnit: 创建轮询单元失败");

        if (!App::_loopUnits[_layer]) App::_loopUnits[_layer] = new std::list<LoopUnit*>();
        App::_loopUnits[_layer]->push_back(this);
        return --(App::_loopUnits[_layer]->cend());
    }

    LoopUnit::~LoopUnit()
    {
        App::_loopUnits[_layer]->erase(_selfApp);
        if (App::_loopUnits[_layer]->empty())
        {
            delete App::_loopUnits[_layer];
            App::_loopUnits[_layer] = nullptr;
        }
    }
}

/// @brief 入口函数, 不要覆写该入口函数, 请用 init(int argc, char* argv[]) 和 quit() 函数进行初始化和销毁
/// @param argc 参数数量
/// @param argv 参数内容
/// @return exitCode
int main(int argc, char* argv[])
{
    try
    {
        onInit(argc, argv);
        XD::App::init(XD::xdWndInitConf_wndName);
#ifndef CC_NO_PLATFORM_SPECIAL_INIT
        XD::platformSpecialInit();
#endif
        onStart(argc, argv);
        bool done = false;
        while (!done) XD::App::onUpdate(done);
        onClose();
        XD::App::onDestroy();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        system("pause");
    }

    return 0;
}

#pragma clang diagnostic pop