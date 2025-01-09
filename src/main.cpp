#include "monitor.hpp"
#include <Windows.h>
#include <stdexcept>
#include <iostream>

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


/// @brief Thread function to read from pipe instance
/// @param instance The named pipe instance to read from
/// @return 
auto WINAPI handle_instance(HANDLE instance) -> DWORD
{
    char buffer[monitor::BUFSIZE];
    DWORD read;
    
    while (true) {
        auto success = ReadFile(
            instance,
            buffer,
            sizeof(buffer),
            &read,
            NULL
        );

        if (!success) {
            break;
        }

        std::cout << "Read: " << buffer << std::endl;
    }

    CloseHandle(instance);
    return 0;
}


/// @brief Waits for a client on the named pipe, then creates a thread to handle the client
/// @param instance 
/// @return Created thread handle
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
/// @return 
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

