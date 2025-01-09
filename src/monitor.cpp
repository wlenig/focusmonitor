#include "monitor.hpp"
#include <Windows.h>
#include <stdexcept>

HHOOK hook;

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

    DWORD mode = PIPE_READMODE_MESSAGE;
    SetNamedPipeHandleState(
        pipe,
        &mode,
        NULL,
        NULL
    );

    return pipe;
}


auto pipe_close(HANDLE pipe)
{
    return CloseHandle(pipe);
}


auto pipe_write(HANDLE pipe, const char* msg)
{
    DWORD written;
    return WriteFile(
        pipe,
        msg,
        strlen(msg),
        &written,
        NULL
    );
}


auto write_name() -> void
{
    auto pipe = pipe_setup();
 
    char name[MAX_PATH];
    GetModuleFileNameA(NULL, name, sizeof(name));
    name[MAX_PATH - 1] = '\0';

    pipe_write(pipe, name);
    pipe_close(pipe);
}


extern "C" __declspec(dllexport) LRESULT CALLBACK CBTProc(
  int    nCode,
  WPARAM wParam,
  LPARAM lParam
) {
    if (nCode == HCBT_SETFOCUS) {
        // TODO: evaluate the current cost of opening a pipe on demand
        //       versus the trade off of opening many pipe instances
        //       (i.e. one per process)
        write_name();
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) auto monitor::InstallHook(HINSTANCE self) -> bool
{
    return SetWindowsHookEx(WH_CBT, CBTProc, self, 0) != NULL;
}
