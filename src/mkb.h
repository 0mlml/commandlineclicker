#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#elif defined(__linux__)
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <unistd.h>

Display *display;
Window root;

void init_linux();
void cleanup_linux();
#endif

void mouseMove(int x, int y);
void mouseDown(int button);
void mouseUp(int button);
void keyDown(char key);
void keyUp(char key);