#include <Windows.h>
#include <stdexcept>
#include <iostream>

void handle_event(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD idEventThread,
    DWORD dwmsEventTime
) {
    std::cout << hwnd << std::endl;
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
}