#include <Windows.h>

namespace monitor
{
    constexpr auto PIPE_NAME = L"\\\\.\\pipe\\FocusMonitorPipe";
    constexpr auto BUFSIZE = 256;
    
    extern "C" __declspec(dllexport) auto InstallHook(HINSTANCE self) -> bool;
}

