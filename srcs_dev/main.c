#include "../include/voicemeeter_bypass.h"

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef void (*open_box_function_t)(uint64_t, uint64_t, uint64_t);
typedef struct {
    open_box_function_t func;  // Pointer to the function to call
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
} remote_call_args_t;

DWORD WINAPI open_license_box_wrapper(LPVOID lp_param) {
	remote_call_args_t *args = (remote_call_args_t *)lp_param;
	args->func(args->arg1, args->arg2, args->arg3);
	return 0;
}

DWORD WINAPI TestFunction(LPVOID param) {
	(void)param;
	MessageBoxA(NULL, "Hello from the remote thread!", "Test", MB_OK);
    Sleep(1000); // Sleep for 1 second to simulate work
    return 0;
}
int main() {
	printf("Start\n");
	

	DWORD voicemeeter64_pid = find_pid_by_process_name("voicemeeter8x64.exe");
	if (voicemeeter64_pid == 0)
	{
		make_log("[ERROR] Voicemeeter process not found.");
		return 1;
	}

	HANDLE voicemeeter64_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, voicemeeter64_pid);
	if (voicemeeter64_handle == NULL)
	{
		make_log("[ERROR] Failed to open Voicemeeter process handle.");
		return 1;
	}


	uint64_t first_arg_value = 0;
	if (ReadProcessMemory(voicemeeter64_handle, (void *)(get_process_base_address(voicemeeter64_pid) + OPEN_BOX_FIRST_ARG_VALUE_OFFSET), &first_arg_value, sizeof(uint64_t), NULL) == 0) {
		make_log("[ERROR] Failed to read first arg value: %d", GetLastError());
		return 1;
	}

	printf("First arg value: %llX\n", first_arg_value);

	// open_box_function_t open_box_function = (open_box_function_t)(get_process_base_address(voicemeeter64_pid) + OPEN_BOX_FUNCTION_OFFSET);
	// remote_call_args_t remote_args = {
	// 	.func = open_box_function,
	// 	.arg1 = first_arg_value,
	// 	.arg2 = OPEN_BOX_SECOND_ARG_LICENSE_ID,  // Replace arg2 with the appropriate value
	// 	.arg3 = 0   // Replace arg3 with the appropriate value
	// };

	// LPVOID remote_args_ptr = VirtualAllocEx(voicemeeter64_handle, NULL, sizeof(remote_call_args_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	// if (remote_args_ptr == NULL) {
	// 	make_log("[ERROR] Failed to allocate memory in remote process: %d", GetLastError());
	// 	return 1;
	// }

	// if (WriteProcessMemory(voicemeeter64_handle, remote_args_ptr, &remote_args, sizeof(remote_call_args_t), NULL) == 0) {
	// 	make_log("[ERROR] Failed to write remote args to remote process: %d", GetLastError());
	// 	VirtualFreeEx(voicemeeter64_handle, remote_args_ptr, 0, MEM_RELEASE);
	// 	return 1;
	// }

	// printf("Calling open box function...\n");
	// printf("Open box function: %p\n", open_box_function);


	// HANDLE thread = CreateRemoteThread(voicemeeter64_handle, NULL, 0,
    //                                open_license_box_wrapper,  // Function to call
    //                                NULL,                                // Pointer to arguments
    //                                0, NULL);
	// if (thread == NULL) {
	// 	make_log("[ERROR] Failed to create remote thread: %d", GetLastError());
	// 	VirtualFreeEx(voicemeeter64_handle, remote_args_ptr, 0, MEM_RELEASE);
	// 	return 1;
	// }
	// WaitForSingleObject(thread, INFINITE);  // Wait for the thread to finish
	// CloseHandle(thread);
	// VirtualFreeEx(voicemeeter64_handle, remote_args_ptr, 0, MEM_RELEASE);


	HANDLE hThread = CreateRemoteThread(voicemeeter64_handle, NULL, 0, TestFunction, NULL, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create remote thread. Error: %ld\n", GetLastError());
        CloseHandle(voicemeeter64_handle);
        return 1;
    }

    // Wait for the thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    CloseHandle(hThread);
    CloseHandle(voicemeeter64_handle);

    printf("Remote thread executed successfully.\n");


	return 0;


	t_registration_window_controls controls = fetch_registration_window_controls();

	if (controls.mail_input == NULL || controls.registration_code_input == NULL || controls.activate_license_button == NULL)
		return 1;

	DWORD_PTR voicemeeter64_base_address = get_process_base_address(voicemeeter64_pid);
	
	char mail_text[256];
	SendMessage(controls.mail_input, WM_GETTEXT, sizeof(mail_text), (LPARAM)mail_text);
	trim(mail_text);
	if (FORCE_SET_DEFAULT_EMAIL || strlen(mail_text) == 0) {
		make_log("Setting email to: %s", DEFAULT_EMAIL);
		if (send_message_to_handle(controls.mail_input, WM_SETTEXT, 0, (LPARAM)DEFAULT_EMAIL) == 0) {
			make_log("[ERROR] Failed to set email.");
			return 1;
		}
	}


	int continue_debug = 1;
	DEBUG_EVENT debug_event;
	CONTEXT context;
	int retry = 0;
	void *breakpoint_address = (void *)(voicemeeter64_base_address + VOICEMEETER_BREAKPOINT_RELATIVE_ADDRESS);
	DWORD64 expected_code = 0;

	if (DebugActiveProcess(voicemeeter64_pid) == 0) {
		make_log("[ERROR] Failed to attach debugger to Voicemeeter process: %d", GetLastError());
		return 1;
	}

	while (continue_debug) {
		if (WaitForDebugEvent(&debug_event, VOICEMEETER_DEBUG_TIMEOUT) == 0) {
			int error = GetLastError();
			if (error == ERROR_SEM_TIMEOUT) {
				make_log("[WARNING] Debug event timeout, try to restart Voicemeeter.");
				if (retry >= VOICEMEETER_DEBUG_RETRY_COUNT)
					return 1;
				
				retry++;
				if (send_post_message_to_handle(controls.activate_license_button, BM_CLICK, 0, 0) == 0)
					make_log("[ERROR] Failed to click activate license button.");
				continue;
			}
			else {
				make_log("[ERROR] Failed to wait for debug event: %d, try to restart Voicemeeter.", GetLastError());
				return 1;
			}
		}

		if (debug_event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
			if (set_breakpoint(voicemeeter64_handle, breakpoint_address) != 0)
				return 1;

			make_log("Breakpoint is set, pressing activate license button.");
			if (send_post_message_to_handle(controls.activate_license_button, BM_CLICK, 0, 0) == 0)
				make_log("[ERROR] Failed to click activate license button.");
		}
		else if (debug_event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
			if (debug_event.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT) {
				HANDLE h_thread = OpenThread(THREAD_ALL_ACCESS, FALSE, debug_event.dwThreadId);
				if (h_thread == NULL) {
					make_log("[ERROR] Failed to open thread: %d", GetLastError());
					ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE);
					continue;
				}

				context.ContextFlags = CONTEXT_FULL;
				if (GetThreadContext(h_thread, &context) == 0) {
					make_log("[ERROR] Failed to get thread context: %d", GetLastError());
					CloseHandle(h_thread);
					ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE);
					continue;
				}

				if (context.Rip == (DWORD_PTR)breakpoint_address + 1) { // breakpoint_address + 1 because the original instruction is 1 byte long, so rip will be at the next instruction
					make_log("Breakpoint hit at address: %p", breakpoint_address);
					expected_code = context.Rax;
					make_log("Expected code: %llX", expected_code);
					if (remove_breakpoint(voicemeeter64_handle, breakpoint_address) != 0) {
						make_log("[ERROR] Failed to remove breakpoint after finding code.");
						return 1;
					}
					context.Rip--; // go back to the original instruction
					context.Dr7 = 0; // Clear debug register so that app doesn't crash when we resume
					// We could also set rbx to expected value here, but we wouls still need to input the code after that (on restart / reopen license window)
					if (SetThreadContext(h_thread, &context) == 0) {
						make_log("[ERROR] Failed to set thread context: %d", GetLastError());
						CloseHandle(h_thread);
						ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE);
						continue;
					}

					continue_debug = 0;
				}
				if (CloseHandle(h_thread) == 0) {
					make_log("[ERROR] Failed to close thread handle: %d", GetLastError());
					return 1;
				}
			}
		}
		ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE);
	}

	if (DebugActiveProcessStop(voicemeeter64_pid) == 0) {
		make_log("[ERROR] Failed to detach debugger from Voicemeeter process: %d", GetLastError());
		return 1;
	}

	HWND failed_window_handle = NULL;
	if (wait_for_window(ACTIVATION_FAILED_WINODW_TITLE, &failed_window_handle, WAIT_FOR_RESPONSE_WINDOW_TIMEOUT, VOICEMEETER_WINODW_POLL_INTERVAL) != 0) {
		make_log("[WARNING] Failed to find activation failed window. Will try to find the activation success one");
		HWND success_window_handle = NULL;
		if (wait_for_window(ACTIVATION_SUCCESS_WINODW_TITLE, &success_window_handle, WAIT_FOR_RESPONSE_WINDOW_TIMEOUT, VOICEMEETER_WINODW_POLL_INTERVAL) != 0) {
			make_log("[ERROR] Also failed to find activation success window.");
			return 1;
		}
		
		make_log("Activation success window found. Code is already correct, closing window...");
		if (send_message_to_handle(success_window_handle, WM_CLOSE, 0, 0) != 0) {
			make_log("[ERROR] Failed to close activation success window.");
			return 1;
		}
	}

	// instantly close the fail window and input the code, then press the button again, then wait for the success window
	if (send_message_to_handle(failed_window_handle, WM_CLOSE, 0, 0) != 0) {
		make_log("[ERROR] Failed to close activation failed window.");
		return 1;
	}

	char code[17];
	sprintf(code, "%llX", expected_code);

	make_log("Entering code: %s", code);
	if (send_message_to_handle(controls.registration_code_input, WM_SETTEXT, 0, (LPARAM)code) == 0) {
		make_log("[ERROR] Failed to enter registration code.");
		return 1;
	}

	retry = 0;
	while (retry < VOICEMEETER_DEBUG_RETRY_COUNT) {
		make_log("Pressing activate license button...");
		if (send_post_message_to_handle(controls.activate_license_button, BM_CLICK, 0, 0) != 0)
			break;
		retry++;
		make_log("[ERROR] Failed to click activate license button, retrying... (attempt %d/%d)", retry, VOICEMEETER_DEBUG_RETRY_COUNT);
	}

	HWND success_window_handle = NULL;
	if (wait_for_window(ACTIVATION_SUCCESS_WINODW_TITLE, &success_window_handle, WAIT_FOR_RESPONSE_WINDOW_TIMEOUT, VOICEMEETER_WINODW_POLL_INTERVAL) != 0) {
		make_log("[ERROR] Failed to find activation success window after entering code.");
		return 1;
	}

	if (send_message_to_handle(success_window_handle, WM_CLOSE, 0, 0) != 0) {
		make_log("[WARNING] Failed to close activation success window.");
		// dont return this time since it still worked
	}

	make_log("Voicemeeter Bypass successfully completed.");
	return 0;
}