#include "mkb.h"
#ifdef _WIN32

void mouseMoveProportional(float x, float y)
{
  x *= 65535;
  y *= 65535;

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

void mouseMove(int x, int y)
{
  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);
  int absoluteX = (x * 65535 + screenWidth / 2) / (screenWidth - 1);
  int absoluteY = (y * 65535 + screenHeight / 2) / (screenHeight - 1);

  INPUT input;
  input.type = INPUT_MOUSE;
  input.mi.dx = absoluteX;
  input.mi.dy = absoluteY;
  input.mi.mouseData = 0;
  input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
  input.mi.time = 0;
  input.mi.dwExtraInfo = 0;

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

MousePos getMousePos()
{
  POINT p;
  GetCursorPos(&p);
  return (MousePos){p.x, p.y};
}

#elif defined(__linux__)

Display *display;
Window root;
XIM im;
XIC ic;

Display *get_display()
{
  return display;
}

Window get_window()
{
  return root;
}

XIM get_input_method()
{
  return im;
}

XIC get_input_context()
{
  return ic;
}

void init_linux()
{
  if (!XInitThreads())
  {
    fprintf(stderr, "Failed to initialize Xlib multithreading support.\n");
    exit(1);
  }

  display = XOpenDisplay(NULL);
  if (display == NULL)
  {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }

  int screen = DefaultScreen(display);
  XSetWindowAttributes attrs;
  attrs.override_redirect = True; // Set override_redirect attribute to True
  root = XCreateWindow(display, RootWindow(display, screen), 10, 10, 1, 1, 0, CopyFromParent, InputOnly, CopyFromParent, CWOverrideRedirect, &attrs);

  XSelectInput(display, root, KeyPressMask);

  im = XOpenIM(display, NULL, NULL, NULL);
  if (im == NULL)
  {
    fprintf(stderr, "Cannot open input method\n");
    exit(1);
  }

  ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, root, XNFocusWindow, root, NULL);
  if (ic == NULL)
  {
    fprintf(stderr, "Cannot create input context\n");
    exit(1);
  }

  XMapWindow(display, root);
  XFlush(display);

  XGrabKeyboard(display, root, true, GrabModeAsync, GrabModeAsync, CurrentTime);
}

void cleanup_linux()
{
  XCloseDisplay(display);
  XUngrabKeyboard(display, CurrentTime);
  XDestroyIC(ic);
  XCloseIM(im);
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

MousePos getMousePos()
{
  int x, y;
  Window child;
  XQueryPointer(display, root, &child, &child, &x, &y, &x, &y, NULL);
  return (MousePos){x, y};
}

#endif
