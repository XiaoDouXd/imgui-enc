#include "_/__app_caller.hpp"
#include "app.h"

#include "ccexce.h"
#include "entrance.h"
#include "imgui_impl_sdl.h"
#include "imguiMgr.h"
#include <iostream>
#include "ui/base/wndMgr.h"

#include "IMGS_TEST_ICON_BMP.h"

namespace CC
{
    uint8_t App::getEventSwitch(const EventSwitch& switchType)
    {
        static size_t typeIdx; typeIdx = (size_t)switchType;
        if (_inst->eventSwitch.size() <= typeIdx) return UINT8_MAX;
        return _inst->eventSwitch[typeIdx];
    }

    void App::setEventSwitch(const EventSwitch& switchType, const uint8_t& state)
    {
        static size_t typeIdx; typeIdx = (size_t)switchType;
        if (_inst->eventSwitch.size() <= typeIdx)
            _inst->eventSwitch.resize(typeIdx + 1, UINT8_MAX);
        _inst->eventSwitch[typeIdx] = state;
    }

    int App::getH() { return _inst->h; }

    int App::getW() { return _inst->w; }

    const clock_t& App::getStartTimePoint()
    {
        return _inst->startClock;
    }

    SDL_Window* App::getWHandle() { return _inst->window; }

    void App::setBGColor(const ImVec4& color) { _inst->bgColor = color; }

    void App::quit() { _inst->willQuit = true; }

    const App::WndEventData& App::event() { return *_eventStateInst; }
}

namespace CC
{
    static inline bool isWindowEvent(const SDL_Event& event)
    {
        return event.type >= SDL_EventType::SDL_WINDOWEVENT_FIRST && event.type <= SDL_EventType::SDL_WINDOWEVENT_LAST;
    }

    static inline bool isEvent(const SDL_Event& event, const SDL_EventType& from, const SDL_EventType& to)
    {
        return event.type >= from && event.type <= to;
    }

    std::unique_ptr<App::WndData> App::_inst = nullptr;
    std::unique_ptr<App::WndEventData> App::_eventStateInst = nullptr;

    // -----------------------------------------------------------------------------------------

    void App::init(const char* wndName)
    {
        if (!_inst) _inst = std::make_unique<WndData>();
        if (!_eventStateInst) _eventStateInst = std::make_unique<WndEventData>();

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0)
            throw Exce(__LINE__, __FILE__, "CC::App: SDL 初始化失败");
        if (SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1") != SDL_TRUE)
            throw Exce(__LINE__, __FILE__, "CC::App: SDL 输入法设置失败");
        TimeMgr::init();
        StaticEventMgr::init();

        _inst->startClock = TimeMgr::now();

        // 初始化 imgui 上下文
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // 绑定到 vulkan 实例
        SDL_WindowFlags wndFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI /*| SDL_WINDOW_RESIZABLE/*| SDL_WINDOW_BORDERLESS*/);
#ifdef CC_WAIT_TIME
        _inst->window = SDL_CreateWindow(wndName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CC_WINDOW_LODING_WIDTH, CC_WINDOW_LODING_HEIGHT, wndFlags);
        setEventSwitch(EventSwitch::ResizeWindow, 0);
#else
        _inst->window = SDL_CreateWindow(wndName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CC_WINDOW_DEFAULT_WIDTH - 1, CC_WINDOW_DEFAULT_HEIGHT, wndFlags);
#endif
        int w, h;
        SDL_GetWindowSize(_inst->window, &w, &h);

        auto iconSurf = SDL_RWFromConstMem((void*) CC::RC::CC_WINDOW_ICON.data(), CC::RC::CC_WINDOW_ICON.size());
        SDL_SetWindowIcon(_inst->window, nullptr);
                uint32_t extensionsCount = 0;
        SDL_Vulkan_GetInstanceExtensions(_inst->window, &extensionsCount, NULL);
        const char** extensions = new const char*[extensionsCount];
        SDL_Vulkan_GetInstanceExtensions(_inst->window, &extensionsCount, extensions);
        VulkanMgr::init(extensions, extensionsCount);

        if (SDL_Vulkan_CreateSurface(_inst->window, VulkanMgr::getInst(), &(_inst->surface)) == 0)
            throw Exce(__LINE__, __FILE__, "CC::App: SDL Surface 创建失败");
        _inst->w = w; _inst->h = h;
        ImguiMgr::init(_inst->window, _inst->surface, _inst->w, _inst->h);
        WndMgr::init();

#ifndef CC_WAIT_TIME
        setEventSwitch(EventSwitch::ResizeWindow, UINT8_MAX);
        ImguiMgr::disableLoading();
#endif
        delete[] extensions;

        // 初始化窗口检测事件
        checkInit();
    }

    void App::updateHead(bool& quit)
    {
        checkFrame();

        // 处理事件
        // 设置 io.WantCaptureMouse, io.WantCaptureKeyboard 让 imgui 使用自定义输入
        // - io.WantCaptureMouse 为 true 时, 不再处理鼠标输入事件
        // - io.WantCaptureKeyboard 为 true 时, 不再捕获键盘输入事件
        static SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                quit = true;
            if (isWindowEvent(event) && event.window.type == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(_inst->window))
                quit = true;
            static int w, h;
            // if (isWindowEvent(event) && event.window.type == SDL_WINDOWEVENT_SIZE_CHANGED && event.window.windowID == SDL_GetWindowID(_inst->window))
            // {
            //     // 由于 DefWindowProc 会在重设大小时阻塞程序运行 这里不使用 winApi 提供的窗口大小重设方法
            //     SDL_GetWindowSize(_inst->window, &w, &h);
            //     ImguiMgr::getIO().DisplaySize = {(float)w, (float)h};
            // }
            checkEvent(event);
        }

        // 重设交换链大小
        ImguiMgr::preRender(_inst->window, _inst->w, _inst->h);
        TimeMgr::refresh();
    }

    void App::updateTail()
    {
        WndMgr::update();
        ImguiMgr::render(_inst->bgColor);
        SDL_GetWindowSize(_inst->window, &(_inst->w), &(_inst->h));

#ifdef CC_WAIT_TIME
        if (ImguiMgr::notLoadingFinished() && !_inst->firstFrame &&
            (TimeMgr::now() - _inst->startClock) > (CC_WAIT_TIME * 1000))
        {
            static int x, y;
            SDL_SetWindowSize(_inst->window, CC_WINDOW_DEFAULT_WIDTH, CC_WINDOW_DEFAULT_HEIGHT);
            SDL_GetWindowPosition(_inst->window, &x, &y);
            SDL_SetWindowPosition(_inst->window,
                x - ((int)(CC_WINDOW_DEFAULT_WIDTH - CC_WINDOW_LODING_WIDTH) >> 1),
                y - ((int)(CC_WINDOW_DEFAULT_HEIGHT - CC_WINDOW_LODING_HEIGHT) >> 1));
            setEventSwitch(EventSwitch::ResizeWindow, UINT8_MAX);
            ImguiMgr::finishLoading();
        }
#endif
        if (_inst->firstFrame)
        {
#ifdef CC_WAIT_TIME
            SDL_SetWindowSize(_inst->window, CC_WINDOW_LODING_WIDTH, CC_WINDOW_LODING_HEIGHT);
#else
            SDL_SetWindowSize(_inst->window, CC_WINDOW_DEFAULT_WIDTH, CC_WINDOW_DEFAULT_HEIGHT);
#endif
            _inst->firstFrame = false;
        }
    }

    void App::onUpdate(bool& quit)
    {
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

    void App::onDestroy()
    {
        auto err = vkDeviceWaitIdle(CC::VulkanMgr::getDev());
        CC::VulkanMgr::checkVkResultCtype(err);

        WndMgr::destory();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        ImguiMgr::destroy();
        VulkanMgr::destroy();
        TimeMgr::destory();

        SDL_DestroyWindow(_inst->window);
        SDL_Quit();

        _inst.reset();
    }

    // -----------------------------------------------------------------------------------------

    std::array<std::list<LoopUnit*>*, CC_MAX_LOOP_LAYER_IDX + 1> App::_loopUnits = {};

    LoopUnit::LoopUnit(LoopLayer layer) : _layer((uint8_t)layer), _selfApp(registerSelf())
    {
        if (!App::_inst) throw Exce(__LINE__, __FILE__, "CC::LoopUnit: App 未初始化");
        App::_inst->newLoopUnit = true;
    }

    std::list<LoopUnit*>::const_iterator LoopUnit::registerSelf()
    {
        if (_layer < 0 || _layer > CC_MAX_LOOP_LAYER_IDX)
            throw Exce(__LINE__, __FILE__, "CC::LoopUnit: 创建轮询单元失败");

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
        CC::__app_caller::init(CC_WINDOW_NAME);
        init(argc, argv);
        bool done = false;
        while (!done) CC::__app_caller::onUpdate(done);
        quit();
        CC::__app_caller::onDestroy();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        system("pause");
    }

    return 0;
}
