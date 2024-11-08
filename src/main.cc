#include "MainWindow.h"
#include <Windows.h>

uintptr_t g_hInst = 0;

int WINAPI wWinMain(HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PWSTR pCmdLine,
                    int nCmdShow) try {
  g_hInst = (uintptr_t)hInstance;
  auto win = IMainWindow::create();
  if (!win) {
    return 1;
  }
  auto res = win->run();
  win = nullptr;

  return res;
} catch (std::exception &e) {
  MessageBoxA(NULL, e.what(), "Error", MB_ICONERROR | MB_OK);
  return 1;
}