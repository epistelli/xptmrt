#include <Windows.h>

ULONGLONG WINAPI impl_GetTickCount64()
{
    return GetTickCount();
}