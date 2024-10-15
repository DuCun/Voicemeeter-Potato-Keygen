#include "../include/voicemeeter_bypass.h"

DWORD findPidByName(const char *name)
{
    HANDLE h;
    PROCESSENTRY32 singleProcess;
    h = CreateToolhelp32Snapshot( //takes a snapshot of specified processes
        TH32CS_SNAPPROCESS, //get all processes
        0); //ignored for SNAPPROCESS

    singleProcess.dwSize = sizeof(PROCESSENTRY32);

    do
    {
        if (strcmp(singleProcess.szExeFile, name) == 0)
        {
            DWORD pid = singleProcess.th32ProcessID;
            CloseHandle(h);
            return pid;
        }

    } while (Process32Next(h, &singleProcess));

    CloseHandle(h);

    return 0;
}

DWORD getProcessBaseAddress(DWORD pid)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &moduleEntry))
    {
        CloseHandle(hSnapshot);
        return (DWORD)moduleEntry.modBaseAddr; // Return base address
    }

    CloseHandle(hSnapshot);
    return 0;
}