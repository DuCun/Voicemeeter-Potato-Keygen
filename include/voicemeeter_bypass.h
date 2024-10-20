#pragma once

#include <windows.h>
#include <tlhelp32.h>

// (*(BASE + VOICEMEETER_TIMER_STRUCT_PTR_OFFSET)) + VOICEMEETER_TIMER_OFFSET = Timer value
#define VOICEMEETER_BANANA_VERSION "3.1.1.3"
#define VOICEMEETER_TIMER_STRUCT_PTR_OFFSET 0x003B7BF8 // Offset to the pointer to the struct containing the timer value relative from base address
#define VOICEMEETER_TIMER_OFFSET 0x10D48 // Offset to the timer value relative to the timer struct
#define VOICEMEETER_BYPASS_DEBUG 0
#define VOICEMEETER_BYPASS_LOG_FILE "VoicemeeeterBypass.log"
#define WAIT_VOICEMEETER_PROCESS_FOR 120000 // ms, max time to wait for voicemeeter to start
#define WAIT_VOICEMEETER_PROCESS_RETRY_INTERVAL 500 // ms, interval to check for voicemeeter process 
#define REGISTRATION_WINDOW_TITLE "About / Registration info..."
#define MAX_WAIT_REGISTRATION_WINDOW_OPEN 3000 // ms, max time to wait for registration window to open
#define MAX_WAIT_TO_CLOSE_REGISTRATION_WINDOW 3000 // ms, max time to wait for registration window to close
#define RETRY_TIME_REGISTRATION_WINDOW 50 // ms, interval to check for registration window to close
#define VOICEMEETER_POTATO_WINDOW_NAME "Voicemeeter Potato"
#define VOICEMEETER_PROCESS_NAME "voicemeeter8x64.exe"
#define MAIL_INPUT_CONTROL_ID 250
#define REGISTRATION_CODE_INPUT_CONTROL_ID 251
#define ACTIVATE_LICENSE_BUTTON_CONTROL_ID 260
#define VOICEMEETER_DEBUG_TIMEOUT 200 // ms, probably means app froze or button press didn't work, so we can set low timeout since button press should be almost instant
#define VOICEMEETER_DEBUG_RETRY_COUNT 10 // retry x times to press the button (only on timeout, other error will instantly quit)
#define VOICEMEETER_BREAKPOINT_RELATIVE_ADDRESS 0x94706 // original instruction is <sete sil> after comparing expected code with entered code
#define WAIT_FOR_RESPONSE_WINDOW_TIMEOUT 1000 // ms, time for response winodw (activation failed or success) to apepar, this should be instant after pressing the button so low timeout
#define ACTIVATION_FAILED_WINODW_TITLE "Activation Failed"
#define ACTIVATION_SUCCESS_WINODW_TITLE "Activation Success"
#define VOICEMEETER_WINODW_POLL_INTERVAL 50 // ms, used in wait_for_window function
#define DEFAULT_EMAIL "pouet@pouet.pouet"
#define FORCE_SET_DEFAULT_EMAIL 0 // if true, even if mail is entered it will be set to default email

typedef struct s_breakpoint
{
    unsigned char original_byte;
    LPVOID address;
    struct s_breakpoint *next;
} t_breakpoint;

typedef struct s_registration_window_controls
{
    HWND mail_input;
    HWND registration_code_input;
    HWND activate_license_button;
} t_registration_window_controls;

// memory_helper.cpp
DWORD find_pid_by_process_name(const char *name);
DWORD_PTR get_process_base_address(DWORD pid);
int remove_breakpoint(HANDLE process_handle, LPVOID address);
int set_breakpoint(HANDLE process_handle, LPVOID address);

// log.c
void make_log(const char *format, ...);

// utils.c
unsigned int get_ms_between_filetimes(FILETIME *start, FILETIME *end);
int wait_for_window(LPCSTR windowTitle, HWND *hwnd, unsigned int timeout, unsigned int pollInterval);
void trim(char *str);

// close_window.c
int close_window(const char *window_title);

// controls.c
t_registration_window_controls fetch_registration_window_controls();
LRESULT send_message_to_handle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT send_post_message_to_handle(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);