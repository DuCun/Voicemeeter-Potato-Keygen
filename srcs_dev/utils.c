#include "../include/voicemeeter_bypass.h"

unsigned int get_ms_between_filetimes(FILETIME *start, FILETIME *end)
{
    ULARGE_INTEGER start_64, end_64;
    start_64.LowPart = start->dwLowDateTime;
    start_64.HighPart = start->dwHighDateTime;
    end_64.LowPart = end->dwLowDateTime;
    end_64.HighPart = end->dwHighDateTime;
    return (unsigned int)((end_64.QuadPart - start_64.QuadPart) / 10000);
}

int wait_for_window(LPCSTR windowTitle, HWND *hwnd, unsigned int timeout, unsigned int pollInterval) {
    // if hwnd is not null, the handle will be stored in it, otherwise it will be ignored
    HWND window = NULL;

    FILETIME start_filetime, current_filetime;
    GetSystemTimeAsFileTime(&start_filetime);
    GetSystemTimeAsFileTime(&current_filetime);
    while (get_ms_between_filetimes(&start_filetime, &current_filetime) < timeout) {
        window = FindWindow(NULL, windowTitle);
        if (window != NULL) {
            if (hwnd != NULL) {
                *hwnd = window;
            }
            return 0;
        }

        Sleep(pollInterval);
        GetSystemTimeAsFileTime(&current_filetime);
    }

    if (hwnd != NULL) {
        *hwnd = NULL;
    }
    return 1;
}

void trim(char *str) {
    // Trim leading and trailing whitespace
    char *start = str;
    char *end;

    while (isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == 0) {
        str[0] = '\0';
        return;
    }

    end = start + strlen(start) - 1;

    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    *(end + 1) = '\0';

    // Move the trimmed string back to the beginning of the buffer
    memmove(str, start, strlen(start) + 1);
}
