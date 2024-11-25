#pragma once

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHARED_RESULT "/shresult"
#define SHARED_MUTEX "/shmutex"

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
	return open(filename, O_RDONLY) != -1;
}

size_t getSize(const char* filename) {
	struct stat st;
	stat(filename, &st);
	return st.st_size;
}

void err(void) {
	fprintf(stderr, "ERRNO = %d\n", errno);
}
