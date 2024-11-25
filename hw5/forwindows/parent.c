#include "utils.h"

int main(int argc, char* argv[]) {
	checkArguments(argc, argv);

	size_t fileSize = getSize(argv[1]);
	size_t threadsNum = atoi(argv[2]);

	if(threadsNum > fileSize / 2) {
		threadsNum = fileSize / 2;
		if(processNum == 0 && filesize) {
			processNum = 1;
		}
		fprintf(stderr, "You probably specified to much threads\n");
		fprintf(stderr, "Threads number updated to %zu\n", threadsNum);
	}

	size_t partLen = fileSize / threadsNum;
	printf("fileSize = %llu, threadsNum = %llu, partLen = %llu\n", fileSize, threadsNum, partLen);

	char* buff = (char*)calloc(fileSize, sizeof(char));
	FILE* inp;
	if((inp = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Failed to read %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	if(fread(buff, sizeof(char), fileSize, inp) != fileSize) {
		fprintf(stderr, "Failed during reading from %s\n", argv[1]);

		free(buff);

		exit(EXIT_FAILURE);
	}
	fclose(inp);

	char name[100];
	size_t currOffset = 0;
	size_t len = partLen;
	for(size_t i = 0; i < threadsNum; ++i) {
		if(i == threadsNum - 1) {
			len = fileSize - (partLen * i);
		}
		sprintf(name, "%llu\0", i);
		FILE* fp = fopen(name, "w");
		if(fp == NULL) {
			fprintf(stderr, "Failed to create %llu file\n", i);

			free(buff);

			exit(EXIT_FAILURE);
		}

		for(size_t k = currOffset; k < (currOffset + len); ++k) {
			fprintf(fp, "%c", buff[k]);
		}

		fclose(fp);
		currOffset += len;
	}
	fclose(inp);
	free(buff);

	STARTUPINFO si;
	GetStartupInfo(&si);

	PROCESS_INFORMATION* pis =
		(PROCESS_INFORMATION*)calloc(threadsNum, sizeof(PROCESS_INFORMATION));

	for(size_t i = 0; i < threadsNum; ++i) {
		sprintf(name, "child.exe %llu\0", i);
		if(!CreateProcess("child.exe", name, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pis[i])) {
			fprintf(stderr, "Failed to create process\n");

			free(pis);

			exit(EXIT_FAILURE);
		}
	}

	for(size_t i = 0; i < threadsNum; ++i) {
		if(WaitForSingleObject(pis[i].hProcess, INFINITE) != WAIT_OBJECT_0) {
			fprintf(stderr, "Failed WaitFSO\n");

			free(pis);

			exit(EXIT_FAILURE);
		}
	}

	DWORD rescode;
	for(size_t i = 0; i < threadsNum; ++i) {
		GetExitCodeProcess(pis[i].hProcess, &rescode);
		if(rescode != 0) {
			fprintf(stderr, "pis[%llu] failed\n", i);

			free(pis);

			exit(EXIT_FAILURE);
		}
	}

	free(pis);

	int ans;
	int tmp;
	for(size_t i = 0; i < threadsNum; ++i) {
		sprintf(name, "%llu.out\0", i);
		FILE* fp = fopen(name, "r");
		fscanf(fp, "%d", &tmp);
		fclose(fp);
		ans += tmp;
	}

	printf("Res = %d\n", ans);

	return 0;
}
