#include "utils.h"

void runShared(char* argv[]);
void runPipe(char* argv[]);

int main(int argc, char* argv[]) {
	if(strcmp(argv[1], "--shared") == 0) {
		runShared(argv);
	}
	else {
		runPipe(argv);
	}
	exit(EXIT_SUCCESS);
}

void runShared(char* argv[]) {
	size_t offset = atoi(argv[2]);
	size_t partlen = atoi(argv[3]);
	size_t filesize = atoi(argv[4]);

	HANDLE hMem = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHMEM);
	HANDLE hRes = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHRES);
	HANDLE hMut = OpenMutex(MUTEX_ALL_ACCESS, FALSE, SHMUT);
	char* mem = MapViewOfFile(hMem, FILE_MAP_ALL_ACCESS, 0, 0, filesize);
	int* res = MapViewOfFile(hRes, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
	
	int locans = 0;
	for(size_t i = 0; i < partlen; ++i) {
		if(mem[i + offset] <= 31 || mem[i + offset] == 127) {
			++locans;
		}
	}

	WaitForSingleObject(hMut, INFINITE);
	*res += locans;
	ReleaseMutex(hMut);
	sleep(6000);
	return;
}

void runPipe(char* argv[]) {
	size_t size = atoi(argv[2]);

	char c;
	int ans = 0;
	for(size_t i = 0; i < size; ++i) {
		scanf("%c", &c);
		if(c <= 31 || c == 127) {
			++ans;
		}
	}
	printf("%d\n", ans);

	return;
}
