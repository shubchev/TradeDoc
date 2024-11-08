#include "MainWindow.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI wWinMain(HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PWSTR pCmdLine,
                    int nCmdShow) try {
  g_hInst = hInstance;
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