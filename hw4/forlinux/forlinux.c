#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define max(a, b)               \
	({                          \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a > _b ? _a : _b;      \
	})

#define min(a, b)               \
	({                          \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a < _b ? _a : _b;      \
	})

int checkNum(const char*);
void print(const int, const char*, const int);
int file_exists(const char*);
size_t getSize(const char*);

int main(int argc, char* argv[]) {
	if(argc < 3) {
		print(STDERR_FILENO, "You must provide 3 argumets\n", -1);
		return 1;
	}
	if(!checkNum(argv[1])) {
		print(STDERR_FILENO, "Second argument must be int\n", -1);
		return 1;
	}
	size_t expected = atoi(argv[1]);
	if(!file_exists(argv[2])) {
		print(STDERR_FILENO, "File doesn't exist\n", -1);
		return 1;
	}
	if(getSize(argv[2]) < expected) {
		print(STDERR_FILENO, "You requested to much...\n", -1);
		return 1;
	}

	size_t buffsize = 100;
	char* buff = (char*)calloc(buffsize, sizeof(char));
	size_t curr = 0;
	int fd = open(argv[2], O_RDONLY);
	while(curr < expected) {
		size_t gotsize = min(buffsize, expected - curr);
		read(fd, buff, gotsize);
		print(STDOUT_FILENO, buff, gotsize);
		curr += gotsize;
	}

	free(buff);
	close(fd);

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
	write(fd, buff, (len == -1 ? strlen(buff) : len));
}

int file_exists(const char* filename) {
	return open(filename, O_RDONLY) != -1;
}

size_t getSize(const char* filename) {
	struct stat st;
	stat(filename, &st);
	return st.st_size;
}
