#ifndef _NTDLL_H
#define _NTDLL_H

extern "C"
{

typedef long NTSTATUS;

#define NT_SUCCESS(Status)  (((NTSTATUS)(Status)) >= 0)

#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWCH   Buffer;
} UNICODE_STRING;

typedef struct _OBJECT_NAME_INFORMATION {
    UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

enum OBJECT_INFORMATION_CLASS 
{
    ObjectBasicInformation, 
    ObjectNameInformation,
    ObjectTypeInformation, 
    ObjectAllInformation, 
    ObjectDataInformation
};

NTSTATUS NTAPI NtQueryObject(
  HANDLE                   Handle,
  OBJECT_INFORMATION_CLASS ObjectInformationClass,
  PVOID                    ObjectInformation,
  ULONG                    ObjectInformationLength,
  PULONG                   ReturnLength
);

ULONG
NTAPI
RtlNtStatusToDosError (
   _In_ NTSTATUS Status
   );

}

#endif // _NTDLL_H