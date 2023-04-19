#include "mkb.h"
#ifdef _WIN32

void mouseMove(int x, int y)
{
  INPUT input;
  input.type = INPUT_MOUSE;
  input.mi.dx = x;
  input.mi.dy = y;
  input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
  input.mi.mouseData = 0;
  input.mi.dwExtraInfo = 0;
  input.mi.time = 0;

  SendInput(1, &input, sizeof(INPUT));
}

void mouseDown(int button)
{
  INPUT input;
  input.type = INPUT_MOUSE;
  input.mi.dx = 0;
  input.mi.dy = 0;
  input.mi.dwFlags =
      (button == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
  input.mi.mouseData = 0;
  input.mi.dwExtraInfo = 0;
  input.mi.time = 0;

  SendInput(1, &input, sizeof(INPUT));
}

void mouseUp(int button)
{
  INPUT input;
  input.type = INPUT_MOUSE;
  input.mi.dx = 0;
  input.mi.dy = 0;
  input.mi.dwFlags = (button == 0) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
  input.mi.mouseData = 0;
  input.mi.dwExtraInfo = 0;
  input.mi.time = 0;

  SendInput(1, &input, sizeof(INPUT));
}

void keyDown(char key)
{
  INPUT input;
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = VkKeyScanA(key);
  input.ki.wScan = 0;
  input.ki.dwFlags = 0;
  input.ki.dwExtraInfo = 0;
  input.ki.time = 0;

  SendInput(1, &input, sizeof(INPUT));
}

void keyUp(char key)
{
  INPUT input;
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = VkKeyScanA(key);
  input.ki.wScan = 0;
  input.ki.dwFlags = KEYEVENTF_KEYUP;
  input.ki.dwExtraInfo = 0;
  input.ki.time = 0;

  SendInput(1, &input, sizeof(INPUT));
}

#elif defined(__linux__)

void init_linux()
{
  display = XOpenDisplay(NULL);
  if (display == NULL)
  {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }
  root = DefaultRootWindow(display);
  XGrabKeyboard(display, root, true, GrabModeAsync, GrabModeAsync, CurrentTime);
}

void cleanup_linux()
{
  XCloseDisplay(display);
  XUngrabKeyboard(display, CurrentTime);
}

void mouseMove(int x, int y)
{
  XTestFakeMotionEvent(display, -1, x, y, CurrentTime);
  XFlush(display);
}

void mouseDown(int button)
{
  XTestFakeButtonEvent(display, button + 1, True, CurrentTime);
  XFlush(display);
}

void mouseUp(int button)
{
  XTestFakeButtonEvent(display, button + 1, False, CurrentTime);
  XFlush(display);
}

void keyDown(char key)
{
  KeyCode keyCode = XKeysymToKeycode(display, key);
  XTestFakeKeyEvent(display, keyCode, True, 0);
  XFlush(display);
}

void keyUp(char key)
{
  KeyCode keyCode = XKeysymToKeycode(display, key);
  XTestFakeKeyEvent(display, keyCode, False, 0);
  XFlush(display);
}

#endif