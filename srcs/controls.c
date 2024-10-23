#include "../include/voicemeeter_bypass.h"

BOOL CALLBACK _enum_registration_window_childs(HWND hwnd, LPARAM lParam) {
    char class_name[256];
    char window_text[256];
    int id;

    GetClassName(hwnd, class_name, sizeof(class_name));
    GetWindowText(hwnd, window_text, sizeof(window_text));
    id = GetDlgCtrlID(hwnd);

    if (id == MAIL_INPUT_CONTROL_ID) {
        ((t_registration_window_controls *)lParam)->mail_input = hwnd;
    } else if (id == REGISTRATION_CODE_INPUT_CONTROL_ID) {
        ((t_registration_window_controls *)lParam)->registration_code_input = hwnd;
    } else if (id == ACTIVATE_LICENSE_BUTTON_CONTROL_ID) {
        ((t_registration_window_controls *)lParam)->activate_license_button = hwnd;
    } else {
        make_log("[WARNING] Unknown control found in registration window (class: %s, text: %s, ID: %d", class_name, window_text, id);
    }

    return TRUE; // Continue enumeration
}

t_registration_window_controls fetch_registration_window_controls(LPCSTR registration_window_title) {
    t_registration_window_controls registration_window_controls = {0};

    HWND window_hwnd = FindWindow(NULL, registration_window_title);
    if (window_hwnd == NULL) {
        make_log("[ERROR] Registration window not found.");
        return registration_window_controls;
    }

    EnumChildWindows(window_hwnd, _enum_registration_window_childs, (LPARAM)&registration_window_controls);

    if (registration_window_controls.mail_input == NULL || registration_window_controls.registration_code_input == NULL || registration_window_controls.activate_license_button == NULL) {
        make_log("[ERROR] Failed to find all controls in registration window:");
        if (registration_window_controls.mail_input == NULL) {
            make_log("\t- Mail input not found.");
        }
        if (registration_window_controls.registration_code_input == NULL) {
            make_log("\t- Registration code input not found.");
        }
        if (registration_window_controls.activate_license_button == NULL) {
            make_log("\t- Activate license button not found.");
        }
        return registration_window_controls;
    }

    return registration_window_controls;
}

// return value depends on message
LRESULT send_message_to_handle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    return SendMessage(hwnd, message, wParam, lParam);
}

LRESULT send_post_message_to_handle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    return PostMessage(hwnd, message, wParam, lParam);
}

void click_on_window_1_1(HWND window_handle) {
	// SetForegroundWindow(window_handle);
	LPARAM lparam = MAKELPARAM(1, 1); // coordinates are relative to the window
    PostMessage(window_handle, WM_LBUTTONDOWN, MK_LBUTTON, lparam);
    PostMessage(window_handle, WM_LBUTTONUP, MK_LBUTTON, lparam);
}

BOOL CALLBACK _enum_active_popup_menu(HWND hwnd, LPARAM lParam) {
    char class_name[256];
    HWND *popup_menu_handle = (HWND*)lParam;

    GetClassName(hwnd, class_name, sizeof(class_name));

    // https://learn.microsoft.com/en-us/windows/win32/winauto/pop-up-menu
    // The window class name for a pop-up menu is "#32768".
    if (strcmp(class_name, "#32768") == 0) {
        DWORD menu_process_id;
        if (GetWindowThreadProcessId(hwnd, &menu_process_id) == 0) {
            make_log("[ERROR] Failed to get thread ID for popup menu.");
            return FALSE;
        }
        DWORD voicemeeter_pid = find_pid_by_process_name(VOICEMEETER32_PROCESS_NAME);
        if (voicemeeter_pid == 0) {
            voicemeeter_pid = find_pid_by_process_name(VOICEMEETER64_PROCESS_NAME);
            if (voicemeeter_pid == 0) {
                make_log("[ERROR] Voicemeeter Potato process not found.");
                return FALSE;
            }
        }

        if (menu_process_id != voicemeeter_pid) {
            make_log("[WARNING] Found popup menu, but it does not belong to Voicemeeter, skipping.");
            return TRUE;
        }

        make_log("Found popup menu belonging to Voicemeeter\n");
        if (*popup_menu_handle != NULL)
        {
            make_log("[ERROR] Multiple popup menus belonging to Voicemeeter found, aborting.");
            *popup_menu_handle = NULL;
            return FALSE;
        } else {
            *popup_menu_handle = hwnd;
        }
    }
    return TRUE;
}

HWND find_active_popup_menu() {
    HWND popup_menu_handle = NULL;
    HWND window_handle = FindWindow(NULL, VOICEMEETER_POTATO_WINDOW_NAME);
    if (window_handle == NULL) {
        make_log("[ERROR] Voicemeeter Potato window not found.");
        return NULL;
    }

    EnumWindows(_enum_active_popup_menu, (LPARAM)&popup_menu_handle);

    if (popup_menu_handle == NULL) {
        make_log("[ERROR] Failed to find active popup menu.");
    }

    return popup_menu_handle;
}

int click_popup_menu_item(HWND popup_menu_window_handle, LPCSTR menu_item_text) {
     HMENU hMenu = (HMENU)SendMessage(popup_menu_window_handle, MN_GETHMENU, 0, 0);
    if (hMenu == NULL) {
        make_log("[ERROR] Failed to get menu handle from popup menu.");
        return 1;
    }

    int menu_item_count = GetMenuItemCount(hMenu);
    if (menu_item_count == -1) {
        make_log("[ERROR] Failed to get menu item count from popup menu.");
        return 1;
    }

    // make_log("Menu item count: %d", menu_item_count);
    char menu_item_text_buffer[256];
    for (int i = 0; i < menu_item_count; i++) {
        if (GetMenuString(hMenu, i, menu_item_text_buffer, sizeof(menu_item_text_buffer), MF_BYPOSITION) == 0) { // not string item (should use GetMenuItemInfo but it keeps returning 0)
            continue;
        }

        if (strcmp(menu_item_text_buffer, menu_item_text) == 0) {
            make_log("Found registration menu item.");
            // We cannot use sendmessage to click it, so find the position and click on the center of butotn, the window should already be on top anyways
            RECT menu_item_rect = {0};
            if (GetMenuItemRect(popup_menu_window_handle, hMenu, i, &menu_item_rect) == 0) {
                make_log("[ERROR] Failed to get menu item rect.");
                return 1;
            }
            int x = menu_item_rect.left + (menu_item_rect.right - menu_item_rect.left) / 2;
            int y = menu_item_rect.top + (menu_item_rect.bottom - menu_item_rect.top) / 2;
            // save cursor position
            POINT cursor_pos;
            int cursor_pos_saved = 0;
            if (GetCursorPos(&cursor_pos) == 0) {
                make_log("[ERROR] Failed to get current cursor position, cannot restore it after clicking.");
            } else {
                cursor_pos_saved = 1;
            }
            if (SetCursorPos(x, y) == 0) {
                make_log("[ERROR] Failed to set cursor position.");
                return 1;
            }
            if (cursor_pos_saved) {
                // restore cursor position
                if (SetCursorPos(cursor_pos.x, cursor_pos.y) == 0) {
                    make_log("[ERROR] Failed to restore cursor position.");
                }
            }
            PostMessage(popup_menu_window_handle, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
            PostMessage(popup_menu_window_handle, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(x, y));
            return 0;
        }
    }
    return 0;
}