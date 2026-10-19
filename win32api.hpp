#pragma once

// Target Version: Windows10+
#ifndef WINVER
#define WINVER 0x0A00
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#ifndef UNICODE
#define UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

// modern looks for the UI
#pragma comment(linker, "\"/manifestdependency:type='win32' " \
                        "name='Microsoft.Windows.Common-Controls' " \
                        "version='6.0.0.0' " \
                        "processorArchitecture='*' " \
                        "publicKeyToken='6595b64144ccf1df' " \
                        "language='*'\"")


// https://learn.microsoft.com/en-us/windows/win32/controls/cookbook-overview