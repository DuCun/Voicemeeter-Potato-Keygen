#if defined(CLOSE_REGISTRATION_WINDOW) | defined(CLOSEMAINWINDOW) | defined(CLOSE_DRIVER_ERROR_WINDOW)
// Function to close a specific Voicemeeter window
BOOL closeWindow(const char *windowTitle)
{
    HWND hWnd = FindWindow(NULL, windowTitle);

    if (hWnd == NULL)
    {
        char logMsg[128];
        sprintf(logMsg, "Window (%s) not found.\n", windowTitle);
        makelog(logMsg);
        return FALSE;
    }

    SendMessage(hWnd, WM_CLOSE, 0, 0);
    if (IsWindow(hWnd))
    {
        char logMsg[128];
        sprintf(logMsg, "Failed to close window (%s). Will try to terminate using PID\n", windowTitle);
        makelog(logMsg);

        // using PID also closes voicemeeter ... sad
        // unsigned long pid = 0;
        // GetWindowThreadProcessId(hWnd, &pid);

        // if (pid == 0)
        // {
        //     char logMsg[128];
        //     sprintf(logMsg, "Failed to get PID for window (%s)\n", windowTitle);
        //     makelog(logMsg);
        //     return FALSE;
        // }
        
        // HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        // if (hProcess == NULL)
        // {
        //     char logMsg[128];
        //     sprintf(logMsg, "Failed to open process for window (%s)\n", windowTitle);
        //     makelog(logMsg);
        //     return FALSE;
        // }

        // if (TerminateProcess(hProcess, 0))
        // {
        //     char logMsg[128];
        //     sprintf(logMsg, "Process for window (%s) terminated successfully.\n", windowTitle);
        //     makelog(logMsg);
        // }
        // else
        // {
        //     char logMsg[128];
        //     sprintf(logMsg, "Failed to terminate process for window (%s)\n", windowTitle);
        //     makelog(logMsg);
        //     return FALSE;
        // }


        printf("PID: %li\n", GetWindowThreadProcessId(hWnd, NULL));
        return FALSE;
    }
    else
    {
        char logMsg[128];
        sprintf(logMsg, "Window (%s) closed successfully.\n", windowTitle);
        makelog(logMsg);
        return TRUE;
    }
}
#endif