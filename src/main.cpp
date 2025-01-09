#include "monitor.hpp"
#include <Windows.h>
#include <stdexcept>

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
    // TODO: loop
    auto buffer = new char[monitor::BUFSIZE];
    DWORD read;

    auto success = ReadFile(
        instance,
        buffer,
        sizeof(buffer),
        &read,
        NULL
    );

    if (!success) {
        throw std::runtime_error("Failed to read from named pipe");
    }
}


/// @brief Waits for a client on the named pipe, then creates a thread to handle the client
/// @param instance 
/// @return Created thread handle
auto accept_client(HANDLE instance) -> HANDLE
{
    auto connected = ConnectNamedPipe(instance, NULL)
        ? TRUE
        : (GetLastError() == ERROR_PIPE_CONNECTED);
    
    if (!connected) {
        throw std::runtime_error("Failed to connect to named pipe");
    }

    auto thread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)handle_instance,
        instance,
        NULL,
        NULL
    );

    if (thread == NULL) {
        throw std::runtime_error("Failed to create thread");
    }

    return thread;
}

// TODO: Implement monitor dll to house the hook proc
//       and invoke it from here

int main() {
    while (true) {
        auto instance = create_instance();
        static auto _ = 0;
        accept_client(instance);
    }
}

