#pragma once
#include <windows.h>

enum class AppTheme
{
    Dark = 0,
    Light = 1
};

struct ThemeHelpers
{
    static AppTheme GetAppTheme();
    static void SetImmersiveDarkMode(HWND window, bool enabled);
};