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

#define SHMEM "/shmem"
#define SHRES "/shres"
#define SHMUT "/shmut"

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
	fprintf(stderr, "./main <filepath> <number of threads> [--shared|--pipe]\n");
}

void usage(void) {
	fprintf(stderr, "./main <filepath> <number of threads> [--shared|--pipe]\n");
}

void checkArguments(int argc, char* argv[]) {
	if(argc != 4) {
		fprintf(stderr, "Wrong arguments, you provided %d, and must provide 4\n", argc);
		usage();
		exit(EXIT_FAILURE);
	}
	if(!fileExists(argv[1])) {
		fprintf(stderr, "Specified file doesn't exist\n");
		usage();
		exit(EXIT_FAILURE);
	}
	if(!isNum(argv[2])) {
		fprintf(stderr, "Second argument must be a number\n");
		usage();
		exit(EXIT_FAILURE);
	}
	if(strcmp(argv[3], "--shared") != 0 && strcmp(argv[3], "--pipe") != 0) {
		fprintf(stderr, "Third argument should be '--shared' or '--pipe'\n");
		usage();
		exit(EXIT_FAILURE);
	}
}
