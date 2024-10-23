#include "../include/voicemeeter_bypass.h"

DWORD find_pid_by_process_name(const char *name)
{
    HANDLE h;
    PROCESSENTRY32 singleProcess;
    h = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS, //get all processes
        0); //ignored for SNAPPROCESS

    if (h == INVALID_HANDLE_VALUE)
        return 0;

    singleProcess.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(h, &singleProcess)) {
        do {
            if (_stricmp(singleProcess.szExeFile, name) == 0) {
                DWORD pid = singleProcess.th32ProcessID;
                CloseHandle(h);
                return pid;
            }
        } while (Process32Next(h, &singleProcess));
    }

    CloseHandle(h);
    return 0;
}

DWORD_PTR get_process_base_address(DWORD pid)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &moduleEntry))
    {
        CloseHandle(hSnapshot);
        return (DWORD_PTR)(moduleEntry.modBaseAddr); // Return base address
    }

    CloseHandle(hSnapshot);
    return 0;
}


int is_process_64_bits(HANDLE process) {
    USHORT processMachine = 0;
    USHORT nativeMachine = 0;

    // Load kernel32.dll
    HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
    if (!hKernel32) {
        make_log("[ERROR] Failed to get kernel32.dll handle: %d\n", GetLastError());
        return -1;
    }

    // Retrieve the address of IsWow64Process2
    typedef BOOL(WINAPI *is_wow_64_process2_fn_ptr)(HANDLE, USHORT*, USHORT*);
    // This is an unsafe cast but in this specific case we have no choices (even used in the official documentation https://learn.microsoft.com/fr-fr/windows/win32/api/wow64apiset/nf-wow64apiset-iswow64process)
    // It compiles well without -Wall -Wextra -Werror but with this we can keep the flags and compile without errors
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-function-type"
    is_wow_64_process2_fn_ptr fnIsWow64Process2 = (is_wow_64_process2_fn_ptr)GetProcAddress(hKernel32, "IsWow64Process2");
    #pragma GCC diagnostic pop // Re-enable warnings 

    if (!fnIsWow64Process2) {
        make_log("[WARNING] IsWow64Process2 not available, using IsWow64Process fallback\n");
        // Fallback to the old IsWow64Process if IsWow64Process2 is unavailable (e.g., on older versions of Windows)
        BOOL bIsWow64 = FALSE;
        typedef BOOL(WINAPI *is_wow_64_fn_ptr)(HANDLE, PBOOL);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-function-type"
        is_wow_64_fn_ptr fnIsWow64Process = (is_wow_64_fn_ptr)GetProcAddress(hKernel32, "IsWow64Process");
        #pragma GCC diagnostic pop // Re-enable warnings 

        if (fnIsWow64Process && fnIsWow64Process(process, &bIsWow64)) {
            return bIsWow64 ? 1 : 0; // 1 if 32-bit process, 0 if 64-bit
        } else {
            make_log("[ERROR] Failed to call IsWow64Process: %d\n", GetLastError());
            return -1;
        }
    }

    // Call IsWow64Process2
    if (!fnIsWow64Process2(process, &processMachine, &nativeMachine)) {
        make_log("[ERROR] Failed to call IsWow64Process2: %d\n", GetLastError());
        return -1;
    }

    // If processMachine is IMAGE_FILE_MACHINE_UNKNOWN, the process is not running under WOW64, meaning it's 64-bit.
    // If processMachine is anything else, it's a 32-bit process running under WOW64.
    if (processMachine == IMAGE_FILE_MACHINE_UNKNOWN) {
        return 1; // not running on WOW64, 64-bit process
    } else {
        return 0; // 32-bit process
    }
}