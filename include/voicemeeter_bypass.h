#pragma once

#include <windows.h>
#include <tlhelp32.h>

#define VOICEMEETER_BYPASS_DEBUG 0

#define VOICEMEETER_BANANA_VERSION "3.1.1.3"
#define VOICEMEETER_BYPASS_LOG_FILE "VoicemeeeterBypass.log"
#define REGISTRATION_WINDOW_TITLE "About / Registration info..."
#define VAIO_REGISTRATION_WINDOW_TITLE "Extra VAIO Feature License"
#define VOICEMEETER_POTATO_WINDOW_NAME "Voicemeeter Potato"
#define VOICEMEETER_PROCESS_NAME "voicemeeter8x64.exe"
#define ACTIVATION_FAILED_WINODW_TITLE "Activation Failed"
#define ACTIVATION_SUCCESS_WINODW_TITLE "Activation Success"

#define MAIL_INPUT_CONTROL_ID 250
#define REGISTRATION_CODE_INPUT_CONTROL_ID 251
#define ACTIVATE_LICENSE_BUTTON_CONTROL_ID 260
#define DEFAULT_EMAIL "pouet@pouet.pouet"
#define FORCE_SET_DEFAULT_EMAIL 0 // if true, even if mail is entered it will be set to default email

#define VOICEMEETER_DEBUG_TIMEOUT 200 // ms, probably means app froze or button press didn't work, so we can set low timeout since button press should be almost instant
#define VOICEMEETER_DEBUG_RETRY_COUNT 10 // retry x times to press the button (only on timeout, other error will instantly quit)
#define VOICEMEETER_BREAKPOINT_RELATIVE_ADDRESS 0x94706 // original instruction is <sete sil> after comparing expected code with entered code
#define WAIT_FOR_RESPONSE_WINDOW_TIMEOUT 1000 // ms, time for response winodw (activation failed or success) to apepar, this should be instant after pressing the button so low timeout
#define VOICEMEETER_WINODW_POLL_INTERVAL 50 // ms, used in wait_for_window function
#define VOICEMEETER_CLICK_FUNCTION_BREAKPOINT_OFFSET 0x17EDD4 // on the cmp instruction (which then defines which button was clicked)
#define CLICK_RAX_VALUE_MENU_BUTTON 0x15C
#define POPUP_MENU_REGISTRATION_ELEMENT_NAME "About Box / License..."
#define POPUP_MENU_VAIO_REGISTRATION_ELEMENT_NAME "VAIO Extension License..."

// memory_helper.c
DWORD find_pid_by_process_name(const char *name);
DWORD_PTR get_process_base_address(DWORD pid);


// log.c
void make_log(const char *format, ...);


// utils.c
unsigned int get_ms_between_filetimes(FILETIME *start, FILETIME *end);
int wait_for_window(LPCSTR windowTitle, HWND *hwnd, unsigned int timeout, unsigned int pollInterval);
void trim(char *str);


// close_window.c
int close_window(const char *window_title);


// controls.c
typedef struct s_registration_window_controls
{
    HWND mail_input;
    HWND registration_code_input;
    HWND activate_license_button;
} t_registration_window_controls;

t_registration_window_controls fetch_registration_window_controls();
LRESULT send_message_to_handle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT send_post_message_to_handle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void click_on_window_1_1(HWND window_handle);
HWND find_active_popup_menu();
int click_popup_menu_item(HWND menu_handle, LPCSTR menu_item_text);


// debug.c
typedef struct s_breakpoint
{
    unsigned char original_byte;
    LPVOID address;
    struct s_breakpoint *next;
} t_breakpoint;

int remove_breakpoint(HANDLE process_handle, LPVOID address);
int set_breakpoint(HANDLE process_handle, LPVOID address);
