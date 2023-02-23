#pragma once

#include <array>
#include <SDL.h>

namespace XD
{
    /// @brief 快捷键
    enum class Shortcut : size_t
    {
        Undo = 0,
        Redo,
        ShortcutCount
    };

    struct ShortcutKey
    {
    public:
        SDL_KeyCode key = SDLK_UNKNOWN;
        SDL_Keymod mod1 = SDL_KMOD_NONE;
        SDL_Keymod mod2 = SDL_KMOD_NONE;
        SDL_Keymod mod3 = SDL_KMOD_NONE;
        SDL_Keymod modIgnore = SDL_KMOD_NONE;
        ShortcutKey(SDL_KeyCode key = SDLK_UNKNOWN,
            SDL_Keymod mod1 = SDL_KMOD_NONE,
            SDL_Keymod mod2 = SDL_KMOD_NONE,
            SDL_Keymod mod3 = SDL_KMOD_NONE,
            SDL_Keymod modIgnore = (SDL_Keymod)(0xF000))
        : key(key), mod1(mod1), mod2(mod2), mod3(mod3), modIgnore(modIgnore) {}
    };

    static inline bool isHit(const ShortcutKey& k, SDL_KeyCode keyCode, SDL_Keymod mod)
    {
        if (keyCode != k.key) return false;
        if (k.mod1 && !(k.mod1 & mod)) return false;
        if (k.mod2 && !(k.mod2 & mod)) return false;
        if (k.mod3 && !(k.mod3 & mod)) return false;
        if (k.mod1) mod =(SDL_Keymod)(mod & ~k.mod1);
        if (k.mod2) mod =(SDL_Keymod)(mod & ~k.mod2);
        if (k.mod3) mod =(SDL_Keymod)(mod & ~k.mod3);
        if (mod & ~k.modIgnore) return false;
        return true;
    }

    extern std::array<ShortcutKey, (size_t)Shortcut::ShortcutCount> ShortcutMap;
} // namespace XD
