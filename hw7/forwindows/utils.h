#pragma once

#include <ctype.h>
#include <fcntl.h>
#include <fileapi.h>
#include <heapapi.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define SHMEM "sharedMemory"
#define SHRES "sharedResult"
#define SHMUT "sharedMutex"

int isNum(const char* str);
int fileExists(const char* filename);
size_t getSize(const char* filename);
void err(void);

int isNum(const char* str) {
	size_t len = strlen(str);
	for(size_t i = 0; i < len; ++i) {
		if(!isdigit(str[i])) {
			return 0;
		}
	}
	return 1;
}

int fileExists(const char* filename) {
	DWORD dwAttrib = GetFileAttributes(filename);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

size_t getSize(const char* filename) {
	HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD size = GetFileSize(fileHandle, NULL);
	return size;
}

void usage(void) {
	fprintf(stderr, "./main <filepath> <number of threads> [--shared|--pipe]\n");
}

void checkArguments(int argc, char* argv[]) {
	if(argc != 4) {
		fprintf(stderr, "Wrong arguments, you provided %d, and must provide 4\n", argc);
		usage();
		exit(0);
	}
	if(!fileExists(argv[1])) {
		fprintf(stderr, "Specified file doesn't exist\n");
		usage();
		exit(0);
	}
	if(!isNum(argv[2])) {
		fprintf(stderr, "Second argument must be a number\n");
		usage();
		exit(0);
	}
	if(strcmp(argv[3], "--shared") != 0 && strcmp(argv[3], "--pipe") != 0) {
		fprintf(stderr, "Third argument should be '--shared' or '--pipe'\n");
		usage();
		exit(EXIT_FAILURE);
	}
}
