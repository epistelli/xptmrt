#ifndef _XPTMRT_COMMON_H
#define _XPTMRT_COMMON_H

#define _WINSOCKAPI_ 
#include <Windows.h>
#include <processthreadsapi.h>
#include <ip2string.h>
#include <Winsock2.h>
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