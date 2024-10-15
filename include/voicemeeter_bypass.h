#pragma once

#include <windows.h>
#include <tlhelp32.h>

// (*(BASE + VOICEMEETER_TIMER_STRUCT_PTR_OFFSET)) + VOICEMEETER_TIMER_OFFSET = Timer value
#define VOICEMEETER_BANANA_VERSION "3.1.1.3"
#define VOICEMEETER_TIMER_STRUCT_PTR_OFFSET 0x003B7BF8 // Offset to the pointer to the struct containing the timer value relative from base address
#define VOICEMEETER_TIMER_OFFSET 0x10D48 // Offset to the timer value relative to the timer struct
// #define VOICEMEETER_BYPASS_DEBUG 1
#define WAIT_VOICEMEETER_PROCESS_FOR 120000 // ms, max time to wait for voicemeeter to start
#define WAIT_VOICEMEETER_PROCESS_RETRY_INTERVAL 500 // ms, interval to check for voicemeeter process 

// memory_helper.cpp
DWORD findPidByName(const char *name);
DWORD getProcessBaseAddress(DWORD pid);

// log.c
void make_log(const char *format, ...);

// utils.c
unsigned int get_ms_between_filetimes(FILETIME *start, FILETIME *end);