#include "utils.h"

int main(int argc, char* argv[]) {
	if(argc != 4) {
		fprintf(stderr, "Wrong arguments, you provided %d, and must provide 3\n", argc);
		fprintf(stderr, "./parent <filepath> <offset> <count>\n");
		exit(EXIT_FAILURE);
	}
	if(!fileExists(argv[1])) {
		fprintf(stderr, "Specified file doesn't exist\n");
		fprintf(stderr, "./parent <filepath> <number of porcesses>\n");
		exit(EXIT_FAILURE);
	}
	if(!isNum(argv[2])) {
		fprintf(stderr, "Second argument must be a number\n");
		fprintf(stderr, "./parent <filepath> <number of porcesses>\n");
		exit(EXIT_FAILURE);
	}
	if(!isNum(argv[3])) {
		fprintf(stderr, "Third argument must be a number\n");
		fprintf(stderr, "./parent <filepath> <number of porcesses>\n");
		exit(EXIT_FAILURE);
	}

	int fd = open(argv[1], O_RDONLY);

	if(fd == -1) {
		fprintf(stderr, "Failed to read %s from child process %d\n", argv[1], getpid());
		exit(EXIT_FAILURE);
	}

	int ans = 0;
	size_t offset = atoi(argv[2]);
	size_t len = atoi(argv[3]);
	char* buff = (char*)malloc(len * sizeof(char));

	pread(fd, buff, len, offset);
	for(size_t i = 0; i < len; ++i) {
		if(buff[i] <= 31 || buff[i] == 127) {
			++ans;
		}
	}

	free(buff);
	close(fd);

	int resFD, mutFD;
	if((resFD = shm_open(SHARED_RESULT, O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
		fprintf(stderr, "Failed to open in child %d shared memory for resFD\n", getpid());
		err();
		exit(EXIT_FAILURE);
	}
	if((mutFD = shm_open(SHARED_MUTEX, O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
		fprintf(stderr, "Failed to open in child %d shared memory for mutFD\n", getpid());
		err();
		exit(EXIT_FAILURE);
	}
	int* res;
	pthread_mutex_t* mut;
	if((res = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, resFD, 0)) ==
	   MAP_FAILED) {
		fprintf(stderr, "Failed to map res in child %d\n", getpid());
		err();
		exit(EXIT_FAILURE);
	}
	if((mut = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutFD, 0)) ==
	   MAP_FAILED) {
		fprintf(stderr, "Failed to map mut in child %d\n", getpid());
		err();
		exit(EXIT_FAILURE);
	}

	pthread_mutex_lock(mut);

	*res += ans;

	pthread_mutex_unlock(mut);

	exit(EXIT_SUCCESS);
}
