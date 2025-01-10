#include "monitor.hpp"
#include <Windows.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <mutex>
#include <sstream>

/// @brief Creates a named pipe instance
/// @return Handle to the created named pipe instance
auto create_instance() -> HANDLE
{
    auto instance = CreateNamedPipe(
        monitor::PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        monitor::BUFSIZE,
        monitor::BUFSIZE,
        0,
        NULL
    );

    if (instance == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create named pipe");
    }

    return instance;
}


auto get_current_time()
{
    auto now = std::time(nullptr);
    auto local = std::localtime(&now);

    char buffer[8 + 1]; // strlen("HH:MM:SS") = 8
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local);

    return std::string(buffer);
}


auto log_focus_event(monitor::FocusEvent event)
{
    // Get console handle for changing color
    auto console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Unable to get handle to console");
    }

    // Convenience console color setter
    auto set_color = [&console](auto color) {
        SetConsoleTextAttribute(console, color);
    };

    // this is how this API was designed???
    constexpr auto WHITE = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
    constexpr auto GRAY = FOREGROUND_INTENSITY;

    // Lock to prevent interleaved output
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    auto time = get_current_time();
    
    std::cout << "[" << time << "] Focus changed" << std::endl;
    set_color(GRAY);
    std::cout << std::right << std::setw(16) << "Process ID : " << event.process_id << std::endl;
    std::cout << std::right << std::setw(16) << "Executable : " << event.executable << std::endl;
    std::cout << std::right << std::setw(16) << "Window : " << event.window_name << std::endl;
    set_color(WHITE);
}


/// @brief Thread function to read from pipe instance
/// @param instance The named pipe instance to read from
/// @return 
auto WINAPI handle_instance(HANDLE instance) -> DWORD
{
    auto event = monitor::FocusEvent{};
    DWORD read;
    
    while (true) {
        auto success = ReadFile(
            instance,
            &event,
            sizeof(event),
            &read,
            NULL
        );

        if (success) {
            log_focus_event(event);
        } else {
            break;
        }
    }

    CloseHandle(instance);
    return 0;
}


/// @brief Waits for a client on the named pipe, then creates a thread to handle the client
/// @param instance The named pipe instance to accept a client on
auto accept_client(HANDLE instance) -> void
{
    auto connected = ConnectNamedPipe(instance, NULL)
        ? TRUE
        : (GetLastError() == ERROR_PIPE_CONNECTED);
    
    if (!connected) {
        throw std::runtime_error("Failed to connect to named pipe");
    }
    
    DWORD thread_id;
    auto thread = CreateThread(
        NULL,
        0,
        handle_instance,
        instance,
        NULL,
        &thread_id
    );

    if (thread == NULL) {
        throw std::runtime_error("Failed to create thread");
    }

    CloseHandle(thread);
}

/// @brief Loads monitor module and calls SetWindowsHookEx with the exported CBT procedure
/// @return Whether the hook was successfully installed
auto install_hook()
{
    auto monitor_module = LoadLibrary(L"monitor.dll");
    if (monitor_module == NULL) {
        throw std::runtime_error("Failed to load monitor module");
    }

    auto install_proc = (decltype(&monitor::InstallHook))GetProcAddress(monitor_module, "InstallHook");
    if (!install_proc) {
        throw std::runtime_error("Failed to find install procedure");
    }

    return install_proc(monitor_module);
}


int main() {
    // Creates a named pipe instance, installs hook __once__ (see: static), then waits for a client to connect.
    // Clients connect, a thread is spawned to read their transmitted FocusEvent, before closing.
    // Loops the behavior to continually receieve new FocusEvents
    // TODO: Consider reusing instances rather than spawning new ones and closing old ones; is there a performance benefit?

    try {
        while (true) {
            auto instance = create_instance();
            static auto hook = install_hook();
            accept_client(instance);
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

