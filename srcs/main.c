#include "../include/voicemeeter_bypass.h"

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	make_log("Voicemeeter Bypass started");

	int voicemeeter_pid = 0;
	FILETIME start_filetime, current_filetime;

	GetSystemTimeAsFileTime(&start_filetime);
	GetSystemTimeAsFileTime(&current_filetime);

	while (get_ms_between_filetimes(&start_filetime, &current_filetime) < WAIT_VOICEMEETER_PROCESS_FOR)
	{
		voicemeeter_pid = findPidByName("voicemeeter8.exe");
		if (voicemeeter_pid != 0)
			break;

		#ifdef VOICEMEETER_BYPASS_DEBUG
		make_log("Waiting for Voicemeeter to start... (%d / %d ms elapsed)", get_ms_between_filetimes(&start_filetime, &current_filetime), WAIT_VOICEMEETER_PROCESS_FOR);
		#else
		make_log("Waiting for Voicemeeter to start...");
		#endif

		Sleep(WAIT_VOICEMEETER_PROCESS_RETRY_INTERVAL);
		GetSystemTimeAsFileTime(&current_filetime);
	}

	if (voicemeeter_pid == 0)
	{
		make_log("[ERROR] Voicemeeter process not found after %d ms, aborting.", WAIT_VOICEMEETER_PROCESS_FOR);
		return EXIT_FAILURE;
	}

	make_log("Voicemeeter instance found with PID: %d", voicemeeter_pid);


	HANDLE voiceemeter_handle = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_VM_READ, 0, voicemeeter_pid);
	if (voiceemeter_handle == NULL)
	{
		printf("its null\n");
		make_log("[WARNING] Failed to open Voicemeeter handle, aborting.");
		return EXIT_FAILURE;
	}


	// pointer to the timer struct
	void *timer_struct_ptr_address = (void *)((unsigned int)getProcessBaseAddress(voicemeeter_pid) + VOICEMEETER_TIMER_STRUCT_PTR_OFFSET);


	// the timer struct itself
	void *timer_struct_address = NULL;
	if (ReadProcessMemory(voiceemeter_handle, timer_struct_ptr_address, &timer_struct_address, sizeof(timer_struct_address), NULL) == 0)
	{
		make_log("[ERROR] Failed to read timer struct pointer, aborting.");
		return EXIT_FAILURE;
	}

	void *timer_address = (void *)((unsigned int)timer_struct_address + VOICEMEETER_TIMER_OFFSET);

	// wait for at least x seconds for window to pop, if stil not found, abort
	HWND registration_hwnd = NULL;
	GetSystemTimeAsFileTime(&start_filetime);
	GetSystemTimeAsFileTime(&current_filetime);
	while (get_ms_between_filetimes(&start_filetime, &current_filetime) < MAX_WAIT_REGISTRATION_WINDOW_OPEN)
	{
		registration_hwnd = FindWindow(NULL, REGISTER_INFO_WINDOW_TITLE);
		if (registration_hwnd != NULL)
			break;

		#ifdef VOICEMEETER_BYPASS_DEBUG
		make_log("Waiting for registration window to open... (%d / %d ms elapsed)", get_ms_between_filetimes(&start_filetime, &current_filetime), MAX_WAIT_REGISTRATION_WINDOW_OPEN);
		#endif

		Sleep(RETRY_TIME_REGISTRATION_WINDOW);	
		GetSystemTimeAsFileTime(&current_filetime);
	}
	if (registration_hwnd == NULL)
	{
		make_log("[ERROR] Registration window not found after %d ms, aborting.", MAX_WAIT_REGISTRATION_WINDOW_OPEN);
		return EXIT_FAILURE;
	}

	make_log("Registration window found, try to close it.");

	while (get_ms_between_filetimes(&start_filetime, &current_filetime) < MAX_WAIT_TO_CLOSE_REGISTRATION_WINDOW){
		#ifdef VOICEMEETER_BYPASS_DEBUG
		int timer_value = 0;
		if (ReadProcessMemory(voiceemeter_handle, timer_address, &timer_value, sizeof(timer_value), NULL) == 0)
			make_log("[WARNING] Failed to read timer value.");
		else
			make_log("Timer value: %d", timer_value);
		#endif

		// now just modify it to 0
		// TODO later find update function and call it
		int new_timer_value = 0;
		if (WriteProcessMemory(voiceemeter_handle, timer_address, &new_timer_value, sizeof(new_timer_value), NULL) == 0)
		{
			make_log("[ERROR] Failed to write timer value, retrying.");
			continue;
		}
		make_log("Sucessfully set the timer value to 0.");

		int close_window_res = close_window(REGISTER_INFO_WINDOW_TITLE);
		if (close_window_res == 0)
			break;
		// else if (close_window_res == 2)
		// 	break;
		Sleep(RETRY_TIME_REGISTRATION_WINDOW);
		GetSystemTimeAsFileTime(&current_filetime);
	}

	// this sends close message to the window, sending it to tray if setting is enabled
	close_window(VOICEMEETER_POTATO_WINDOW_NAME);

	make_log("Voicemeeter Bypass successfully completed.");


	return 0;
}