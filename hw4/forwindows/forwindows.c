#include <ctype.h>
#include <fileapi.h>
#include <heapapi.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

int checkNum(const char*);
void print(const int, const char*, const int);
int file_exists(const char*);
size_t getSize(const char*);

int main(int argc, char* argv[]) {
	if(argc < 3) {
		print(1, "You must provide 3 argumets\n", -1);
		return 1;
	}
	if(!checkNum(argv[1])) {
		print(1, "Second argument must be int\n", -1);
		return 1;
	}
	size_t expected = atoi(argv[1]);
	if(!file_exists(argv[2])) {
		print(1, "File doesn't exist\n", -1);
		return 1;
	}
	if(getSize(argv[2]) < expected) {
		print(1, "You requested to much...\n", -1);
		return 1;
	}

	HANDLE hHeap = GetProcessHeap();

	if(hHeap == NULL) {
		print(1, "Getting process heap failed\n", -1);
		return 1;
	}

	size_t buffsize = 100;
	char* buff = (char*)HeapAlloc(hHeap, 0, buffsize);

	if(buff == NULL) {
		print(1, "Failed to alloc buff\n", -1);
		return 1;
	}

	size_t curr = 0;
	HANDLE fileHandle = CreateFileA(argv[2], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL, NULL);
	while(curr < expected) {
		size_t gotsize = min(buffsize, expected - curr);
		ReadFile(fileHandle, buff, gotsize, NULL, NULL);

		print(0, buff, gotsize);
		curr += gotsize;
	}

	HeapFree(hHeap, 0, buff);
	buff = NULL;
	return 0;
}

// stuff...

int checkNum(const char* str) {
	size_t len = strlen(str);
	for(size_t i = 0; i < len; ++i) {
		if(!isdigit(str[i])) {
			return 0;
		}
	}
	return 1;
}

void print(const int fd, const char* buff, const int len) {
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	switch(fd) {
	case 0: out = GetStdHandle(STD_OUTPUT_HANDLE); break;
	case 1: out = GetStdHandle(STD_ERROR_HANDLE); break;
	}

	if(out != NULL && out != INVALID_HANDLE_VALUE) {
		WriteConsoleA(out, buff, (len == -1 ? strlen(buff) : len), NULL, NULL);
	}
	else {
		exit(1);
	}
}

int file_exists(const char* filename) {
	DWORD dwAttrib = GetFileAttributes(filename);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

size_t getSize(const char* filename) {
	HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD size = GetFileSize(fileHandle, NULL);
	return size;
}
