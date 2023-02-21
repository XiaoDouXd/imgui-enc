#pragma once

#include "app.h"
#include "imguiMgr.h"
#include "staticEvent.h"
#include "staticEventMgr.h"
#include "timeMgr.h"
#include "wndMgr.h"
#include "vulkanMgr.h"

// ================================================== 程序参数预定义
namespace XD
{
    extern const uint8_t* xdWndInitConf_iconPngData;
    extern size_t xdWndInitConf_iconPngSize;
    extern const char* xdWndInitConf_wndName;
    extern const char* xdWndInitConf_confFileName;
    extern double xdWndInitConf_waitTime;
    extern int xdWndInitConf_defWndWidth;
    extern int xdWndInitConf_defWndHeight;
    extern int xdWndInitConf_lodingWidth;
    extern int xdWndInitConf_lodingHeight;
}

// ================================================== 需要实现的函数
/// @brief 程序初始化前执行
/// @param argc 参数数量
/// @param argv 传入参数
extern void onInit(int argc, char* argv[]);

/// @brief 程序初始化后执行
/// @param argc 参数数量
/// @param argv 传入参数
extern void onStart(int argc, char* argv[]);

/// @brief 程序退出时执行
extern void onClose();

// ================================================== 不要实现该函数
/// @brief 入口函数 (留用) 不要实现它
extern int main(int argc, char* argv[]);