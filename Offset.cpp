#include <ntddk.h>
#include "Offset.h"

// ntoskrnl.exe�� ������ ������ ������� ��������� �׻� ������ ���� ������ ������.
// ImageName : System
// PID : 4
// PLIST_ENTRY = PID + sizeof(PID)

extern "C"
ULONG CalcPIDOffset()
{
    PEPROCESS peprocess = IoGetCurrentProcess();
    HANDLE pid = PsGetCurrentProcessId();
    PLIST_ENTRY list = NULL;
    int i;

    for (i = 0; i < PAGE_SIZE; i += 4)
    {
        if (*(PHANDLE)((PCHAR)peprocess + i) == pid)
        {
            // PLIST_ENTRY�� PID ������ ��ġ������.
            list = (PLIST_ENTRY)((unsigned char*)peprocess + i + sizeof(HANDLE));
            
            // ��ȿ�� �ּ����� Ȯ���Ѵ�.
            if (MmIsAddressValid(list))
            {
                // ���������� ����� ����Ʈ���� Ȯ�� �� PID �ּҸ� ��ȯ�Ѵ�.
                if (list == list->Flink->Blink)
                {
                    return i;
                }
            }
        }
    }

    return 0;
}

extern "C"
ULONG CalcProcessNameOffset()
{
    // ntoskrn.exe �� EPROCESS ����ü ȹ��
    PEPROCESS ntosKrnl = PsInitialSystemProcess;
    int i = 0;

    for (i = 0; i < PAGE_SIZE; i++)
    {
        if (RtlCompareMemory((PCHAR)ntosKrnl + i, "System", 6) == 6)
        {
            return i;
        }
    }

    return 0;
}