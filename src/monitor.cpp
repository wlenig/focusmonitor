#include "monitor.hpp"
#include <Windows.h>
#include <stdexcept>


auto pipe_setup()
{
    auto pipe = CreateFile(
        monitor::PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    return pipe;
}


auto pipe_close(HANDLE pipe)
{
    return CloseHandle(pipe);
}


auto pipe_write(HANDLE pipe, void* msg, size_t size)
{
    DWORD written;
    return WriteFile(
        pipe,
        msg,
        size,
        &written,
        NULL
    );
}

 
auto write_event(HWND focused) -> void
{
    auto pipe = pipe_setup();
 
    auto event = monitor::FocusEvent{};
    GetModuleFileNameA(NULL, event.executable, sizeof(event.executable));
    GetWindowTextA(focused, event.window_name, sizeof(event.window_name));
    event.process_id = GetCurrentProcessId();

    pipe_write(pipe, &event, sizeof(event));
    pipe_close(pipe);
}


extern "C" __declspec(dllexport) LRESULT CALLBACK CBTProc(
  int    nCode,
  WPARAM wParam,
  LPARAM lParam
) {
    // wParam is handle to window gaining focus
    // lParam is handle to window losing focus

    if (nCode == HCBT_SETFOCUS) {
        // TODO: evaluate the current cost of opening a pipe on demand
        //       versus the trade off of opening many pipe instances
        //       (i.e. one per process)
        write_event((HWND)wParam);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) auto monitor::InstallHook(HINSTANCE self) -> bool
{
    return SetWindowsHookEx(WH_CBT, CBTProc, self, 0) != NULL;
}
