#include <Windows.h>
#include <stdio.h>

int main()
{
    WCHAR Path[MAX_PATH];
    HANDLE hFile = CreateFileW(L"C:\\Windows\\System32\\calc.exe", GENERIC_READ, FILE_SHARE_READ, NULL, 
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    GetFinalPathNameByHandleW(hFile, Path, MAX_PATH, VOLUME_NAME_NT);
    wprintf(L"GetFinalPathNameByHandleW: %s\n", Path);
    CloseHandle(hFile);
    return 0;
}