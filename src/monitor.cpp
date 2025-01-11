#include <Windows.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <Psapi.h>
#include <Shlwapi.h>

struct FocusInfo {
    HWND window;
    DWORD process_id;
    char executable_file[MAX_PATH];
    char window_name[MAX_PATH];
    const char* executable_name;
};

auto get_info(HWND hwnd)
{
    FocusInfo info{};
    info.executable_file[0] = '\0';
    info.window_name[0] = '\0';
    info.executable_name = "";

    if (!GetWindowThreadProcessId(hwnd, &info.process_id)) {
        throw std::runtime_error("Failed to get process ID");
    }

    auto process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, info.process_id);
    if (process == NULL) {
        throw std::runtime_error("Failed to open process");
    }

    if (!GetModuleFileNameExA(process, NULL, info.executable_file, sizeof(info.executable_file))) {
        CloseHandle(process);
        throw std::runtime_error("Failed to get executable name");
    }

    // Get short file name
    info.executable_name = PathFindFileNameA(info.executable_file);

    // Get window title name. Note: not all windows have title names, so we don't check return
    GetWindowTextA(hwnd, info.window_name, sizeof(info.window_name));

    CloseHandle(process);
    return info;
}

auto get_current_time()
{
    auto now = std::time(nullptr);
    auto local = std::localtime(&now);

    char buffer[8 + 1]; // strlen("HH:MM:SS") = 8
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local);

    return std::string(buffer);
}

auto log_info(FocusInfo info)
{
    auto time_str = get_current_time();

    std::cout << "[" << time_str << "] " << info.executable_name << std::endl;
    std::cout << std::setw(16) << std::right << "Process ID : " << info.process_id << std::endl;
    std::cout << std::setw(16) << std::right << "Window title : " << info.window_name << std::endl;
    std::cout << std::setw(16) << std::right << "Executable : " << info.executable_file << std::endl;
    std::cout << std::setw(16) << std::right << "Handle : " << info.window << std::endl;
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
    try {

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
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}