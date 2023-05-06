#pragma once

#include <cstdint>

// ================================================== Pre def

namespace XD {
    extern const uint8_t* xdWndInitConf_iconPngData;
    extern size_t xdWndInitConf_iconPngSize;
    extern const char* xdWndInitConf_wndName;
    extern const char* xdWndInitConf_confFileName;
    extern double xdWndInitConf_waitTime;
    extern int xdWndInitConf_defWndWidth;
    extern int xdWndInitConf_defWndHeight;
    extern int xdWndInitConf_loadingWidth;
    extern int xdWndInitConf_loadingHeight;
}

// ================================================== Interface func
/// @brief init func,
/// called before appBase inited
/// @param argc sys param count
/// @param argv sys params
extern void onInit(int argc, char* argv[]);

/// @brief start func,
/// called after appBase inited
/// @param argc sys param count
/// @param argv sys params
extern void onStart(int argc, char* argv[]);

/// @brief close func,
/// called while quit program
extern void onClose();

// ================================================== Retention
/// @brief this function is implemented by appBase,
/// so don't redefine this in your program
extern int main(int argc, char* argv[]);