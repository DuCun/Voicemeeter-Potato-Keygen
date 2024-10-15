#include <windows.h>
#include <tlhelp32.h> //for CreateToolhelp32Snapshot
#include <stdio.h>
#include <iostream>

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

DWORD GetBaseAddress(DWORD pid) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &moduleEntry)) {
        CloseHandle(hSnapshot);
        return (DWORD)moduleEntry.modBaseAddr; // Return base address
    }

    CloseHandle(hSnapshot);
    return 0;
}

int main()
{
	// std::cout << "test" << std::endl;
	
	int pid = findPidByName("voicemeeter8.exe");

	if (pid)
	{
		printf("found\n");

		HANDLE handleToProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

		if (handleToProcess)
		{
			printf("try memory read\n");
			void *address = (void *)((unsigned int)GetBaseAddress(pid) + 0x003B7BF8);
			DWORD offset = 0x10D48;
			printf("address: %p\n", address);


			unsigned int data = 123456;
			printf("oeoee %d\n", sizeof(unsigned int));
			ReadProcessMemory(handleToProcess, address, &data, sizeof(data), NULL);

			printf("Res: %u\n", data);
			// data + offset gives the address of the value

			void *targetAddress = (void *)(data + offset);
			printf("Target address: %p\n", targetAddress);
			int timer = 0;
			ReadProcessMemory(handleToProcess, targetAddress, &timer, sizeof(timer), NULL);
			printf("Timer: %d\n", timer);

			// now write targetaddress to 0
			int newValue = 0;
			if (WriteProcessMemory(handleToProcess, targetAddress, &newValue, sizeof(newValue), NULL))
			{
				printf("Timer value successfully modified.\n");
			}
			else
			{
				printf("Failed to modify the timer value.\n");
			}

		}
		else
		{
			printf("couldn't open process\n");
		}

	}
	else
	{
		printf("not found\n");
	}

	return 0;
}