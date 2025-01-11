#include <Windows.h>
#include <stdexcept>
#include <iostream>
#include <Psapi.h>

struct FocusInfo {
    DWORD process_id;
    char process_name[MAX_PATH];
};

auto get_info(HWND hwnd)
{
    FocusInfo info{};

    if (!GetWindowThreadProcessId(hwnd, &info.process_id)) {
        throw std::runtime_error("Failed to get process ID");
    }

    auto process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, info.process_id);
    if (process == NULL) {
        throw std::runtime_error("Failed to open process");
    }

    if(!GetModuleFileNameExA(process, NULL, info.process_name, sizeof(info.process_name))) {
        CloseHandle(process);
        throw std::runtime_error("Failed to get file name");
    }

    CloseHandle(process);
    return info;
}

auto log_info(FocusInfo info)
{
    // Prevent duplicates, as the event is often triggered multiple times per focus change
    static DWORD last_pid = 0;
    if (info.process_id == last_pid) {
        return;
    }
    last_pid = info.process_id;

    std::cout << info.process_id << " : " << info.process_name << std::endl;
}

void handle_event(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd, // handle to window being focused
    LONG idObject,
    LONG idChild,
    DWORD idEventThread,
    DWORD dwmsEventTime
) {
    auto info = get_info(hwnd);
    log_info(info);
}

void message_loop()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int main()
{
    auto hook = SetWinEventHook(
        EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS,
        NULL,
        handle_event,
        0,
        0,
        WINEVENT_OUTOFCONTEXT
    );

    if (!hook) {
        throw std::runtime_error("Failed to set event hook");
    }

    message_loop();
    UnhookWinEvent(hook);

    return 0;
}