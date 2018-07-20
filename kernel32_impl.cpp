#include <Windows.h>
#include <processthreadsapi.h>
#include <string.h>
#include "ntdll.h"

#define TRACE(...)
#define FIXME(...)
#define WARN(...)

#define strlenW wcslen
#define strcatW wcscat
#define strncmpiW _wcsnicmp

//
// files
//

// from the Wine project
DWORD WINAPI impl_GetFinalPathNameByHandleW(HANDLE file, LPWSTR path, DWORD charcount, DWORD flags)
{
    WCHAR buffer[sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH + 1];
    OBJECT_NAME_INFORMATION *info = (OBJECT_NAME_INFORMATION*)&buffer;
    WCHAR drive_part[MAX_PATH];
    DWORD drive_part_len = 0;
    NTSTATUS status;
    DWORD result = 0;
    ULONG dummy;
    WCHAR *ptr;

    TRACE( "(%p,%p,%d,%x)\n", file, path, charcount, flags );

    if (flags & ~(FILE_NAME_OPENED | VOLUME_NAME_GUID | VOLUME_NAME_NONE | VOLUME_NAME_NT))
    {
        WARN("Unknown flags: %x\n", flags);
        SetLastError( ERROR_INVALID_PARAMETER );
        return 0;
    }

    /* get object name */
    status = NtQueryObject(file, ObjectNameInformation, &buffer, sizeof(buffer) - sizeof(WCHAR), &dummy );
    if (status != STATUS_SUCCESS)
    {
        SetLastError( RtlNtStatusToDosError( status ) );
        return 0;
    }
    
    if (!info->Name.Buffer)
    {
        SetLastError( ERROR_INVALID_HANDLE );
        return 0;
    }
    
    // convert \Device\HarddiskVolume2 to drive letter
    {
        WCHAR device[MAX_PATH];
        WCHAR dos_drive[3];
        dos_drive[1] = ':';
        dos_drive[2] = 0;
        for (char c = 'A'; c <= 'Z'; c++)
        {
            dos_drive[0] = c;
            if (QueryDosDeviceW(dos_drive, device, MAX_PATH) == 0)
                continue;
            size_t len = wcslen(device);
            if (_wcsnicmp(info->Name.Buffer, device, len) == 0)
            {
                WCHAR temp[MAX_PATH];
                wcscpy(temp, dos_drive);
                wcscpy(&temp[2], &info->Name.Buffer[len]);
                wcscpy(info->Name.Buffer, temp);
                break;
            }
        }
    }

    /* FILE_NAME_OPENED is not supported yet, and would require Wineserver changes */
    if (flags & FILE_NAME_OPENED)
    {
        FIXME("FILE_NAME_OPENED not supported\n");
        flags &= ~FILE_NAME_OPENED;
    }

    /* Get information required for VOLUME_NAME_NONE, VOLUME_NAME_GUID and VOLUME_NAME_NT */
    if (flags == VOLUME_NAME_NONE || flags == VOLUME_NAME_GUID || flags == VOLUME_NAME_NT)
    {
        if (!GetVolumePathNameW( info->Name.Buffer, drive_part, MAX_PATH ))
            return 0;

        drive_part_len = strlenW(drive_part);
        if (!drive_part_len || drive_part_len > strlenW(info->Name.Buffer) ||
                drive_part[drive_part_len-1] != '\\' ||
                strncmpiW( info->Name.Buffer, drive_part, drive_part_len ))
        {
            FIXME("Path %s returned by GetVolumePathNameW does not match file path %s\n",
                debugstr_w(drive_part), debugstr_w(info->Name.Buffer));
            SetLastError( ERROR_GEN_FAILURE );
            return 0;
        }
    }

    if (flags == VOLUME_NAME_NONE)
    {
        ptr    = info->Name.Buffer + drive_part_len - 1;
        result = strlenW(ptr);
        if (result < charcount)
            memcpy(path, ptr, (result + 1) * sizeof(WCHAR));
        else result++;
    }
    else if (flags == VOLUME_NAME_GUID)
    {
        WCHAR volume_prefix[51];

        /* GetVolumeNameForVolumeMountPointW sets error code on failure */
        if (!GetVolumeNameForVolumeMountPointW( drive_part, volume_prefix, 50 ))
            return 0;

        ptr    = info->Name.Buffer + drive_part_len;
        result = strlenW(volume_prefix) + strlenW(ptr);
        if (result < charcount)
        {
            path[0] = 0;
            strcatW(path, volume_prefix);
            strcatW(path, ptr);
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            result++;
        }
    }
    else if (flags == VOLUME_NAME_NT)
    {
        WCHAR nt_prefix[MAX_PATH];

        /* QueryDosDeviceW sets error code on failure */
        drive_part[drive_part_len - 1] = 0;
        if (!QueryDosDeviceW( drive_part, nt_prefix, MAX_PATH ))
            return 0;

        ptr    = info->Name.Buffer + drive_part_len - 1;
        result = strlenW(nt_prefix) + strlenW(ptr);
        if (result < charcount)
        {
            path[0] = 0;
            strcatW(path, nt_prefix);
            strcatW(path, ptr);
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            result++;
        }
    }
    else if (flags == VOLUME_NAME_DOS)
    {
        static const WCHAR dos_prefix[] = {'\\','\\','?','\\', '\0'};

        result = strlenW(dos_prefix) + strlenW(info->Name.Buffer);
        if (result < charcount)
        {
            path[0] = 0;
            strcatW(path, dos_prefix);
            strcatW(path, info->Name.Buffer);
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            result++;
        }
    }
    else
    {
        /* Windows crashes here, but we prefer returning ERROR_INVALID_PARAMETER */
        WARN("Invalid combination of flags: %x\n", flags);
        SetLastError( ERROR_INVALID_PARAMETER );
    }

    return result;
}

// from the Wine project
DWORD WINAPI impl_GetFinalPathNameByHandleA(HANDLE file, LPSTR path, DWORD charcount, DWORD flags)
{
    WCHAR *str;
    DWORD result, len, cp;

    TRACE( "(%p,%p,%d,%x)\n", file, path, charcount, flags);

    len = impl_GetFinalPathNameByHandleW(file, NULL, 0, flags);
    if (len == 0)
        return 0;

    str = (WCHAR *) HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
    if (!str)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    result = impl_GetFinalPathNameByHandleW(file, str, len, flags);
    if (result != len - 1)
    {
        HeapFree(GetProcessHeap(), 0, str);
        WARN("GetFinalPathNameByHandleW failed unexpectedly: %u\n", result);
        return 0;
    }

    cp = !AreFileApisANSI() ? CP_OEMCP : CP_ACP;

    len = WideCharToMultiByte(cp, 0, str, -1, NULL, 0, NULL, NULL);
    if (!len)
    {
        HeapFree(GetProcessHeap(), 0, str);
        WARN("Failed to get multibyte length\n");
        return 0;
    }

    if (charcount < len)
    {
        HeapFree(GetProcessHeap(), 0, str);
        return len - 1;
    }

    len = WideCharToMultiByte(cp, 0, str, -1, path, charcount, NULL, NULL);
    if (!len)
    {
        HeapFree(GetProcessHeap(), 0, str);
        WARN("WideCharToMultiByte failed\n");
        return 0;
    }

    HeapFree(GetProcessHeap(), 0, str);

    return len - 1;
}

//
// processes
//

struct proc_thread_attr
{
    DWORD_PTR attr;
    SIZE_T size;
    void *value;
};

struct _PROC_THREAD_ATTRIBUTE_LIST
{
    DWORD mask;  /* bitmask of items in list */
    DWORD size;  /* max number of items in list */
    DWORD count; /* number of items in list */
    DWORD pad;
    DWORD_PTR unk;
    struct proc_thread_attr attrs[1];
};

// from the Wine project
BOOL WINAPI impl_InitializeProcThreadAttributeList(struct _PROC_THREAD_ATTRIBUTE_LIST *list, DWORD count, DWORD flags, SIZE_T *size)
{
    SIZE_T needed;
    BOOL ret = FALSE;

    TRACE("(%p %d %x %p)\n", list, count, flags, size);

    needed = FIELD_OFFSET(struct _PROC_THREAD_ATTRIBUTE_LIST, attrs[count]);
    if (list && *size >= needed)
    {
        list->mask = 0;
        list->size = count;
        list->count = 0;
        list->unk = 0;
        ret = TRUE;
    }
    else
        SetLastError(ERROR_INSUFFICIENT_BUFFER);

    *size = needed;
    return ret;
}

// from the Wine project
BOOL WINAPI impl_UpdateProcThreadAttribute(struct _PROC_THREAD_ATTRIBUTE_LIST *list,
                                           DWORD flags, DWORD_PTR attr, void *value, SIZE_T size,
                                           void *prev_ret, SIZE_T *size_ret)
{
    DWORD mask;
    struct proc_thread_attr *entry;

    TRACE("(%p %x %08lx %p %ld %p %p)\n", list, flags, attr, value, size, prev_ret, size_ret);

    if (list->count >= list->size)
    {
        SetLastError(ERROR_GEN_FAILURE);
        return FALSE;
    }

    switch (attr)
    {
    case PROC_THREAD_ATTRIBUTE_PARENT_PROCESS:
        if (size != sizeof(HANDLE))
        {
            SetLastError(ERROR_BAD_LENGTH);
            return FALSE;
        }
        break;

    case PROC_THREAD_ATTRIBUTE_HANDLE_LIST:
        if ((size / sizeof(HANDLE)) * sizeof(HANDLE) != size)
        {
            SetLastError(ERROR_BAD_LENGTH);
            return FALSE;
        }
        break;

    case PROC_THREAD_ATTRIBUTE_IDEAL_PROCESSOR:
        if (size != sizeof(PROCESSOR_NUMBER))
        {
            SetLastError(ERROR_BAD_LENGTH);
            return FALSE;
        }
        break;

    default:
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    mask = 1 << (attr & PROC_THREAD_ATTRIBUTE_NUMBER);

    if (list->mask & mask)
    {
        SetLastError(ERROR_OBJECT_NAME_EXISTS);
        return FALSE;
    }

    list->mask |= mask;

    entry = list->attrs + list->count;
    entry->attr = attr;
    entry->size = size;
    entry->value = value;
    list->count++;

    return TRUE;
}

void WINAPI impl_DeleteProcThreadAttributeList(struct _PROC_THREAD_ATTRIBUTE_LIST *)
{
}

//
// misc
//

ULONGLONG WINAPI impl_GetTickCount64()
{
    return GetTickCount();
}