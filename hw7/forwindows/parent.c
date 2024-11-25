#include "utils.h"

void runShared(char* filepath, size_t threadsNum, size_t filesize);
void runPipe(char* filepath, size_t threadsNum, size_t filesize);

int main(int argc, char* argv[]) {
	checkArguments(argc, argv);

	char* filepath = argv[1];
	size_t filesize = getSize(argv[1]);
	size_t threadsNum = atoi(argv[2]);

	if((threadsNum > filesize / 2) || (threadsNum == 0)) {
		threadsNum = filesize / 2;
		if(threadsNum == 0 && filesize) {
			threadsNum = 1;
		}
		fprintf(stderr, "You probably specified to much threads\n");
		fprintf(stderr, "Threads number updated to %zu\n", threadsNum);
	}

	printf("fileSize = %lu, threadsNum = %lu\n", filesize, threadsNum);

	if(strcmp(argv[3], "--shared") == 0) {
		printf("go to shared\n");
		runShared(filepath, threadsNum, filesize);
	}
	else {
		printf("go to pipe\n");
		runPipe(filepath, threadsNum, filesize);
	}

	exit(EXIT_SUCCESS);

	return 0;
}

void runShared(char* filepath, size_t threadsNum, size_t filesize) {
	HANDLE hMem, hRes, hMut;

	// creating
	if((hMem = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, filesize,
								  SHMEM)) == NULL) {
		fprintf(stderr, "Failed to create mapping for mem\n");
		exit(EXIT_FAILURE);
	}
	if((hRes = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int),
								  SHRES)) == NULL) {
		fprintf(stderr, "Failed to create mapping for res\n");
		exit(EXIT_FAILURE);
	}
	if((hMut = CreateMutexA(NULL, FALSE, SHMUT)) == NULL) {
		fprintf(stderr, "Failed to create mutex\n");
		exit(EXIT_FAILURE);
	}

	// mapping
	char* mem = MapViewOfFile(hMem, FILE_MAP_ALL_ACCESS, 0, 0, filesize);
	char* res = MapViewOfFile(hRes, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));

	// initialize shared
	FILE* fp;
	if((fp = fopen(filepath, "r")) == NULL) {
		fprintf(stderr, "Failed to read %s\n", filepath);

		UnmapViewOfFile(mem);
		UnmapViewOfFile(res);
		CloseHandle(hMem);
		CloseHandle(hRes);
		CloseHandle(hMut);

		exit(EXIT_FAILURE);
	}
	fread(mem, sizeof(char), filesize, fp);
	fclose(fp);

	*res = 0;

	size_t partlen = filesize / threadsNum;
	size_t offset = 0;
	char params[200];
	HANDLE* hProcs = (HANDLE*)calloc(threadsNum, sizeof(HANDLE));

	// start processes
	printf("threadsNum %lu, partlen %lu, filesize %lu\n", threadsNum, partlen, filesize);

	for(size_t i = 0; i < threadsNum; ++i) {
		if(i == (threadsNum - 1)) {
			partlen = filesize - (partlen * i);
		}

		sprintf(params, "child.exe --shared %lu %lu %lu\0", offset, partlen, filesize);

		STARTUPINFO si;
		GetStartupInfo(&si);
		PROCESS_INFORMATION pi;

		if(CreateProcess("child.exe", params, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == 0) {
			fprintf(stderr, "Failed to create process\n");

			UnmapViewOfFile(mem);
			UnmapViewOfFile(res);
			CloseHandle(hMem);
			CloseHandle(hRes);
			CloseHandle(hMut);

			exit(EXIT_FAILURE);
		}

		hProcs[i] = pi.hProcess;
		offset += partlen;
	}

	WaitForMultipleObjects(threadsNum, hProcs, TRUE, INFINITE);

	int rescode = 0, code;
	for(size_t i = 0; i < threadsNum; ++i) {
		GetExitCodeProcess(hProcs[i], &code);
		rescode |= code;
	}
	if(code) {
		code = 0;
	}
	else {
		code = 1;
	}

	printf("rescode = %d\n", rescode);
	printf("res = %d\n", *res);

	UnmapViewOfFile(mem);
	UnmapViewOfFile(res);
	CloseHandle(hMem);
	CloseHandle(hRes);
	CloseHandle(hMut);

	return;
}

void InitializeSecurityAttr(LPSECURITY_ATTRIBUTES attr, SECURITY_DESCRIPTOR* sd) {
	attr->nLength = sizeof(SECURITY_ATTRIBUTES);
	attr->bInheritHandle = TRUE;
	InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
	attr->lpSecurityDescriptor = sd;
}

struct ProcPipes {
	HANDLE p11;
	HANDLE p12;
	HANDLE p21;
	HANDLE p22;
};

void runPipe(char* filepath, size_t threadsNum, size_t filesize) {
	size_t offset = 0;
	size_t partlen = filesize / threadsNum;

	HANDLE sin = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE sout = GetStdHandle(STD_OUTPUT_HANDLE);

	struct ProcPipes* pipes = (struct ProcPipes*)calloc(threadsNum, sizeof(struct ProcPipes));
	HANDLE* pHandles = (HANDLE*)calloc(threadsNum, sizeof(HANDLE));

	char params[100];
	for(size_t i = 0; i < threadsNum; ++i) {
		if(i == (threadsNum - 1)) {
			partlen = filesize - (partlen * i);
		}
		SECURITY_ATTRIBUTES attr;
		SECURITY_DESCRIPTOR sd;
		InitializeSecurityAttr(&attr, &sd);
		CreatePipe(&(pipes[i].p11), &(pipes[i].p12), &attr, 0);
		CreatePipe(&(pipes[i].p21), &(pipes[i].p22), &attr, 0);

		// master -> p12  p11 -> slave
		// slave -> p22  p21 -> master

		SetStdHandle(STD_INPUT_HANDLE, pipes[i].p11);
		SetStdHandle(STD_OUTPUT_HANDLE, pipes[i].p22);

		STARTUPINFO si;
		GetStartupInfo(&si);
		PROCESS_INFORMATION pi;

		sprintf(params, "child.exe --pipe %lu\0", partlen);
		if(CreateProcess("child.exe", params, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == 0) {
			fprintf(stderr, "Failed to create process\n");

			free(pipes);
			free(pHandles);

			exit(EXIT_FAILURE);
		}

		pHandles[i] = pi.hProcess;
	}

	SetStdHandle(STD_INPUT_HANDLE, sin);
	SetStdHandle(STD_OUTPUT_HANDLE, sout);

	// open and read file
	FILE* file;
	if((file = fopen(filepath, "r")) == NULL) {
		fprintf(stderr, "Failed to open %s\n", filepath);

		free(pipes);
		free(pHandles);

		exit(EXIT_FAILURE);
	}
	char* buff = (char*)calloc(filesize, sizeof(char));
	if(fread(buff, sizeof(char), filesize, file) != filesize) {
		fprintf(stderr, "Failed to read %lu bytes from %s\n", filesize, filepath);

		free(pipes);
		free(pHandles);

		exit(EXIT_FAILURE);
	}

	fclose(file);

	// master -> p12  p11 -> slave
	// slave -> p22  p21 -> master

	// write to pipes
	partlen = filesize / threadsNum;
	for(size_t i = 0; i < threadsNum; ++i) {
		if(i == (threadsNum - 1)) {
			partlen = filesize - (partlen * i);
		}

		char* part = (char*)calloc(partlen, sizeof(char));
		strncpy(part, buff + offset, partlen);
		WriteFile(pipes[i].p12, part, partlen, NULL, NULL);
		CloseHandle(pipes[i].p12);
		free(part);

		offset += partlen;
	}

	free(buff);

	// read from pipes
	int ans = 0;
	char childAns[100];
	size_t len;
	for(size_t i = 0; i < threadsNum; ++i) {
		ReadFile(pipes[i].p21, childAns, 100, &len, NULL);
		childAns[len] = '\0';
		ans += atoi(childAns);
	}

	// finish processes
	WaitForMultipleObjects(threadsNum, pHandles, TRUE, INFINITE);
	int rescode = 0, code;
	for(size_t i = 0; i < threadsNum; ++i) {
		GetExitCodeProcess(pHandles[i], &code);
		rescode |= code;
	}
	if(rescode) {
		rescode = 0;
	}
	else {
		rescode = 1;
	}

	printf("rescode = %d\n", rescode);
	printf("ans = %d\n", ans);

	free(pipes);
	free(pHandles);

	return;
}
