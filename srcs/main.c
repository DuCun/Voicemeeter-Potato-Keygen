#include "../include/voicemeeter_bypass.h"

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main() {
	t_registration_window_controls controls = fetch_registration_window_controls();
	if (controls.mail_input == NULL || controls.registration_code_input == NULL || controls.activate_license_button == NULL)
		return 1;

	DWORD voicemeeter64_pid = find_pid_by_process_name("voicemeeter8x64.exe");
	if (voicemeeter64_pid == 0)
	{
		make_log("[ERROR] Voicemeeter process not found.");
		return 1;
	}

	HANDLE voicemeeter64_handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, voicemeeter64_pid);
	if (voicemeeter64_handle == NULL)
	{
		make_log("[ERROR] Failed to open Voicemeeter process handle.");
		return 1;
	}

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