#include "entrance.h"
#include "ui/mainPanel.hpp"

namespace CC
{
    /// @brief 主循环启动器
    class MainUnit : public CC::LoopUnit
    {
    public:
        MainUnit() : CC::LoopUnit() {}
        static size_t mainPanelId;

        void start() override
        {
            using namespace CC;
            using namespace CC::UI;
            mainPanelId = WndMgr::open<MainPanel>();
        }

        void update() override
        {
            using namespace CC;
            using namespace CC::UI;
            if (ImguiMgr::notLoadingFinished()) return;
            static bool show_demo_window = true;
            ImGui::ShowDemoWindow(&show_demo_window);

            if (ImguiMgr::getIO().KeysDown[ImGuiKey_Escape])
                App::quit();
        }
    };
    size_t MainUnit::mainPanelId = 0;
    static std::unique_ptr<MainUnit> mainUnit = nullptr;
}

void init(int argc, char* argv[])
{
    CC::mainUnit = std::make_unique<CC::MainUnit>();
}

void quit()
{
    CC::mainUnit.reset();
}