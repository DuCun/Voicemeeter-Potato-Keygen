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

t_registration_window_controls fetch_registration_window_controls() {
    t_registration_window_controls registration_window_controls = {0};

    HWND window_hwnd = FindWindow(NULL, REGISTRATION_WINDOW_TITLE);
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

unsigned __stdcall click_on_window_1_1(void *hwnd) {
	HWND window_hwnd = (HWND)hwnd;
	SetForegroundWindow(window_hwnd);
	LPARAM lparam = MAKELPARAM(1, 1); // coordinates are relative to the window
	PostMessage(window_hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lparam);
	PostMessage(window_hwnd, WM_LBUTTONUP, MK_LBUTTON, lparam);
	return 0;
}