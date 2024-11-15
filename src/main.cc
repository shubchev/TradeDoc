#include <Windows.h>
#define DEF_DEVMODE
#include "MainWindow.h"
#include "resource.h"

uintptr_t g_hInst = 0;

int WINAPI wWinMain(HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PWSTR pCmdLine,
                    int nCmdShow) try {
  g_hInst = (uintptr_t)hInstance;

  auto hModule = GetModuleHandle(NULL);
  auto hFontRes = FindResource(hModule,
                               MAKEINTRESOURCE(IDR_BINARYFONT),
                               "BINARYFONT");
  if (!hFontRes) {
    MessageBoxA(GetDesktopWindow(), "Failed to load GUI font", "Error", MB_ICONERROR | MB_OK);
    return 1;
  }

  auto hRes = LoadResource(hModule, hFontRes);
  auto fontData = LockResource(hRes);
  auto fontDataSize = (size_t)SizeofResource(hModule, hFontRes);
  Array<uint8_t> fd(fontDataSize);
  MemCopy(fd.data(), fontData, min(fontDataSize, fd.size()));

  auto win = IMainWindow::create(fd);
  if (!win) {
    return 1;
  }
  auto res = win->run();
  win = nullptr;

  return res;
} catch (std::exception &e) {
  MessageBoxA(GetDesktopWindow(), e.what(), "Error", MB_ICONERROR | MB_OK);
  return 1;
}