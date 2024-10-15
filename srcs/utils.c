#include "../include/voicemeeter_bypass.h"

unsigned int get_ms_between_filetimes(FILETIME *start, FILETIME *end)
{
    ULARGE_INTEGER start_64, end_64;
    start_64.LowPart = start->dwLowDateTime;
    start_64.HighPart = start->dwHighDateTime;
    end_64.LowPart = end->dwLowDateTime;
    end_64.HighPart = end->dwHighDateTime;
    return (unsigned int)((end_64.QuadPart - start_64.QuadPart) / 10000);
}