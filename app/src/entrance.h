#pragma once

#include "cc.h"

// ================================================== 程序参数预定义
#define CC_WINDOW_ICON CC_RC_IMGS_CLIPTOOLICON_PNG // 窗口图标
#define CC_WINDOW_NAME "🔪🩸💔😨"                 // 窗口名称
#define CC_CONF_FILE_NAME "clipTool.ini"           // 缓存文件名
// #define CC_WAIT_TIME 5                      // 窗口 Loading 界面最小等待时间
#define CC_WINDOW_DEFAULT_WIDTH 640         // 默认窗口宽
#define CC_WINDOW_DEFAULT_HEIGHT 360        // 默认窗口高
#define CC_WINDOW_LODING_WIDTH 512          // Loading 界面宽 和 最小窗口宽
#define CC_WINDOW_LODING_HEIGHT 288         // Loading 界面高 和 最小窗口高

// ================================================== 程序生命周期方法
/// @brief 程序初始化时执行
/// @param argc 参数数量
/// @param argv 传入参数
extern void init(int argc, char* argv[]);

/// @brief 程序退出时执行
extern void quit();