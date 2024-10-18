#include "../include/voicemeeter_bypass.h"

t_breakpoint *breakpoints = NULL;

t_breakpoint *_get_breakpoint_by_address(LPVOID address) {
    t_breakpoint *current = breakpoints;
    while (current != NULL)
    {
        if (current->address == address)
            return current;
        current = current->next;
    }
    return NULL;
}

int remove_breakpoint(HANDLE process_handle, LPVOID address) {
    t_breakpoint *breakpoint = _get_breakpoint_by_address(address);
    if (breakpoint == NULL)
    {
        make_log("[WARNING] Breakpoint not found at address: %p.", address);
        return 1;
    }

    if (WriteProcessMemory(process_handle, address, &breakpoint->original_byte, 1, NULL) == 0)
    {
        make_log("[ERROR] Failed to write original byte back to breakpoint address.");
        return 1;
    }

    t_breakpoint *current = breakpoints;
    t_breakpoint *previous = NULL;
    while (current != NULL)
    {
        if (current == breakpoint)
        {
            if (previous == NULL)
                breakpoints = current->next;
            else
                previous->next = current->next;
            free(current);
            break;
        }
        previous = current;
        current = current->next;
    }

    make_log("Breakpoint removed at address: %p.", address);
    return 0;
}

int set_breakpoint(HANDLE process_handle, LPVOID address) {
    BYTE int3 = 0xCC; // INT 3 instruction (breakpoint)

    t_breakpoint *existing_breakpoint = _get_breakpoint_by_address(address);
    if (existing_breakpoint != NULL)
    {
        make_log("[WARNING] Breakpoint already set at address: %p, replacing it.", address);
        remove_breakpoint(process_handle, address);
    }

    t_breakpoint *new_breakpoint = (t_breakpoint *)malloc(sizeof(t_breakpoint));
    if (new_breakpoint == NULL)
    {
        make_log("[ERROR] Failed to allocate memory for breakpoint data.");
        return 1;
    }
    
    new_breakpoint->address = address;
    new_breakpoint->next = breakpoints;
    breakpoints = new_breakpoint;

    if (ReadProcessMemory(process_handle, address, &new_breakpoint->original_byte, 1, NULL) == 0)
    {
        make_log("[ERROR] Failed to read original byte at breakpoint address.");
        return 1;
    }

    if (WriteProcessMemory(process_handle, address, &int3, 1, NULL) == 0)
    {
        make_log("[ERROR] Failed to write breakpoint instruction.");
        return 1;
    }

    make_log("Breakpoint set at address: %p.", address);

    return 0;
}

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