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