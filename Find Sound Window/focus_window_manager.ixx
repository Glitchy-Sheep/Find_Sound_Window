#include <wtypes.h>

export module focus_window_manager;

// For tricky operations in window enumeration
struct EnumWindowsData {
    DWORD targetProcessId;
    HWND mainWindowHandle;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);

    DWORD currentWindowProcessId;
    GetWindowThreadProcessId(hwnd, &currentWindowProcessId);

    // If the process id matched our handle
    // return its hwnd
    if (currentWindowProcessId == data->targetProcessId) {
        data->mainWindowHandle = hwnd;
        return FALSE;  // Stop enumerating
    }
    return TRUE;  // Continue enumerating
}

// Function to find the main window handle of a process
HWND GetMainWindowHandleByProcessId(DWORD processId)
{
    EnumWindowsData data = { processId, nullptr };
    EnumWindows(EnumWindowsProc, (LPARAM)&data);
    return data.mainWindowHandle;
}

export void FocusMainProcessWindow(DWORD processId) {
    auto hwnd = GetMainWindowHandleByProcessId(processId);
    SetForegroundWindow(hwnd);
}

