#include "../include/voicemeeter_bypass.h"

int close_window(const char *window_title)
{
    HWND hwnd = FindWindow(NULL, window_title);

    if (hwnd == NULL)
    {
        make_log("[WARNING] Cannot close window (%s) (non-existent).", window_title);
        return 0;
    }

    SendMessage(hwnd, WM_CLOSE, 0, 0);
    if (IsWindowVisible(hwnd))
    {
        make_log("[ERROR] Failed to close window (%s).", window_title);
        return 1;
    }

    make_log("Successfully closed window (%s).", window_title);
    return 0;
}