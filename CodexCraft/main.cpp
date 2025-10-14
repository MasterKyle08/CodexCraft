#include "src/platform/windows/Application.h"

using CodexCraft::Platform::Windows::Application;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    Application application(hInstance, nCmdShow);
    return application.Run();
}

int main() {
    return wWinMain(GetModuleHandleW(nullptr), nullptr, nullptr, SW_SHOWDEFAULT);
}

