#include <Windows.h>

namespace monitor
{
    constexpr auto PIPE_NAME = L"\\\\.\\pipe\\FocusMonitorPipe";
    constexpr auto BUFSIZE = 256;
    
    /// @brief Struct used for communicating change of focus
    struct FocusEvent {
        char executable[MAX_PATH];
        char window_name[MAX_PATH];
        DWORD process_id;
    };

    extern "C" __declspec(dllexport) auto InstallHook(HINSTANCE self) -> bool;
}

