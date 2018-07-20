#ifndef _XPTMRT_COMMON_H
#define _XPTMRT_COMMON_H

#include <Windows.h>
#include <processthreadsapi.h>
#include <string.h>
#include "ntdll.h"

// for Wine compatibility

#define TRACE(...)
#define FIXME(...)
#define WARN(...)

#define strlenW (DWORD) wcslen
#define strcatW (DWORD) wcscat
#define strncmpiW (DWORD) _wcsnicmp

#endif // _XPTMRT_COMMON_H