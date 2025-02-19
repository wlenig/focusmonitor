#include <Windows.h>

namespace monitor
{
    struct FocusInfo {
        HWND window;
        DWORD process_id;
        char executable_file[MAX_PATH];
        unsigned executable_name_offset;
        char window_name[MAX_PATH];
    };
}

