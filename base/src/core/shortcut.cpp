#include "shortcut.h"

namespace XD
{
    std::array<ShortcutKey, (size_t)Shortcut::ShortcutCount> ShortcutMap = {
        ShortcutKey(SDLK_z, SDL_KMOD_CTRL),                    // UNDO
        ShortcutKey(SDLK_z, SDL_KMOD_CTRL, SDL_KMOD_SHIFT)     // REDO
    };
} // namespace XD
