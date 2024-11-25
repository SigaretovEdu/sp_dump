#include "utils.h"

struct ThreadParams {
	size_t offset;
	size_t len;
	char* filepath;
};

size_t ans = 0;
char* filepath;
CRITICAL_SECTION cs;

DWORD WINAPI calc(LPVOID args);
int readFrom(char* filepath, size_t offset, size_t len, char* buff);

int main(int argc, char* argv[]) {
	checkArguments(argc, argv);

	filepath = argv[1];
	size_t fileSize = getSize(argv[1]);
	size_t threadsNum = atoi(argv[2]);

	if((threadsNum > fileSize / 2) || (threadsNum == 0)) {
		threadsNum = fileSize / 2;
		if(threadsNum == 0 && fileSize) {
			threadsNum = 1;
		}
		fprintf(stderr, "You probably specified to much threads\n");
		fprintf(stderr, "Threads number updated to %zu\n", threadsNum);
	}

	size_t partLen = fileSize / threadsNum;
	printf("fileSize = %llu, threadsNum = %llu, partLen = %llu\n", fileSize, threadsNum, partLen);

	struct ThreadParams* params =
		(struct ThreadParams*)calloc(threadsNum, sizeof(struct ThreadParams));
	for(size_t thrNum = 0; thrNum < threadsNum; ++thrNum) {
		params[thrNum].offset = thrNum * partLen;
		params[thrNum].len = (thrNum != threadsNum - 1) ? partLen : (fileSize - thrNum * partLen);
		params[thrNum].filepath = argv[1];
	}

	for(size_t thrNum = 0; thrNum < threadsNum; ++thrNum) {
		printf("thrNum = %llu, offset = %llu, len = %llu\n", thrNum, params[thrNum].offset,
			   params[thrNum].len);
	}

	InitializeCriticalSection(&cs);
	HANDLE* hThreads = (HANDLE*)calloc(threadsNum, sizeof(HANDLE));
	for(size_t thrNum = 0; thrNum < threadsNum; ++thrNum) {
		hThreads[thrNum] = CreateThread(NULL, 0, calc, (LPVOID)&params[thrNum], 0, NULL);
	}
	
	WaitForMultipleObjects(threadsNum, hThreads, 1, INFINITE);
	DWORD resCode = 0, code;
	for(size_t thrNum = 0; thrNum < threadsNum; ++thrNum) {
		GetExitCodeThread(hThreads[thrNum], &code);
		resCode |= code;
	}
	if(resCode == 0) {
		resCode = 1;
	}
	else {
		resCode = 0;
	}
	printf("resCode = %ld\n", resCode);

	DeleteCriticalSection(&cs);
	free(hThreads);
	free(params);

	exit(EXIT_SUCCESS);
}

int readFrom(char* filepath, size_t offset, size_t len, char* buff) {
	if(len == 0) {
		return 1;
	}
	FILE* fp = fopen(filepath, "r");
	if(!fp) {
		fprintf(stderr, "Failed to open %s in thread %ld", filepath, GetCurrentThreadId());
		return 1;
	}
	if(fseek(fp, offset, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to fseek to offset %llu in thread %ld", offset,
				GetCurrentThreadId());
		fclose(fp);
		return 1;
	}
	if(fread(buff, 1, len, fp) != len) {
		fprintf(stderr, "Failed to read %llu bytes in thread %ld", len, GetCurrentThreadId());
		fclose(fp);
		return 1;
	}
	fclose(fp);
	return 0;
}

DWORD WINAPI calc(LPVOID args) {
	struct ThreadParams* params = (struct ThreadParams*)args;

	int localAns = 0;
	char* buff = (char*)calloc(params->len, sizeof(char));
	if(readFrom(params->filepath, params->offset, params->len, buff) != 0) {
		free(buff);
		ExitThread(1);
	}
	for(size_t i = 0; i < params->len; ++i) {
		if(buff[i] <= 31 || buff[i] == 127) {
			++localAns;
		}
	}

	printf("threadId = %ld, offset = %llu, len = %llu, localAns = %d\n", GetCurrentThreadId(),
		   params->offset, params->len, localAns);

	EnterCriticalSection(&cs);
	ans += localAns;
	LeaveCriticalSection(&cs);

	sleep(6000);
	ExitThread(0);
}
