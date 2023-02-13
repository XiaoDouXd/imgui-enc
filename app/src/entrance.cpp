#include "core/fileCtrl.h"
#include "entrance.h"

#include "ui/ctrlPanel.hpp"
#include "ui/mainPanel.hpp"
#include "ui/picPanel.hpp"

#include "ui/staticPanelData.h"

namespace CC
{
    /// @brief 主循环启动器
    class MainUnit : public CC::LoopUnit
    {
    public:
        MainUnit() : CC::LoopUnit() {}

        void start() override
        {
            using namespace CC;
            using namespace CC::UI;

            FileCtrl::init();
            ClipCtrl::init();

            WndMgr::open<MainPanel>();
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

    UI::MainPanel_cover* UI::MainPanel_cover::_this = nullptr;
    static std::unique_ptr<MainUnit> mainUnit = nullptr;

    ssize_t UI::curHoveredClip = -1;
    ssize_t UI::curDragedClip = -1;
    std::vector<uint8_t> UI::curSelectedClips = {};
    std::vector<glm::vec4> UI::clipChanges;
    bool UI::curSelectedClipsReset = true;
}

void init(int argc, char* argv[])
{
    CC::mainUnit = std::make_unique<CC::MainUnit>();
}

void quit()
{
    CC::mainUnit.reset();
}