#include "utils.h"

int main(int argc, char* argv[]) {
	if(argc != 3) {
		fprintf(stderr, "Wrong arguments, you provided %d, and must provide 3\n", argc);
		fprintf(stderr, "./parent <filepath> <number of porcesses>\n");
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

	size_t fileSize = getSize(argv[1]);
	size_t processNum = atoi(argv[2]);

	if(processNum > fileSize / 2) {
		processNum = fileSize / 2;
		if(processNum == 0 && filesize) {
			processNum = 1;
		}
		fprintf(stderr, "You probably specified to much processes\n");
		fprintf(stderr, "Process number updated to %zu\n", processNum);
	}

	size_t len = fileSize / processNum;
	size_t offset = 0;
	char p1[33];
	char p2[33];

	int resFD = -1, mutFD = -1;
	if((resFD = shm_open(SHARED_RESULT, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR)) == -1) {
		fprintf(stderr, "Failed to open in parent shared memory for resFD\n");
		err();
		exit(EXIT_FAILURE);
	}
	if((mutFD = shm_open(SHARED_MUTEX, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR)) == -1) {
		fprintf(stderr, "Failed to open in parent shared memory for mutFD\n");
		err();
		exit(EXIT_FAILURE);
	}
	if(ftruncate(resFD, sizeof(int)) != 0) {
		fprintf(stderr, "Failed to truncate memory for resFD\n");
		err();
		exit(EXIT_FAILURE);
	}
	if(ftruncate(mutFD, sizeof(pthread_mutex_t)) != 0) {
		fprintf(stderr, "Failed to truncate memory for mutFD\n");
		err();
		exit(EXIT_FAILURE);
	}
	int* res;
	pthread_mutex_t* mut;
	if((res = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, resFD, 0)) ==
	   MAP_FAILED) {
		fprintf(stderr, "Failed to map res in parent\n");
		err();
		exit(EXIT_FAILURE);
	}
	if((mut = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutFD, 0)) ==
	   MAP_FAILED) {
		fprintf(stderr, "Failed to map mut in parent\n");
		err();
		exit(EXIT_FAILURE);
	}
	close(resFD);
	close(mutFD);

	*res = 0;

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(mut, &attr);
	pid_t pids[1000];

	for(size_t i = 0; i < processNum; ++i) {
		offset = len * i;
		if(i == processNum - 1) {
			len = fileSize - offset;
		}
		sprintf(p1, "%zu", offset);
		sprintf(p2, "%zu", len);

		pid_t child = fork();
		switch(child) {
		case -1: {
			fprintf(stderr, "Failed to create process\n");

			shm_unlink(SHARED_RESULT);
			shm_unlink(SHARED_MUTEX);

			exit(EXIT_FAILURE);
		}
		case 0: {
			char* childArgv[] = {"child", argv[1], p1, p2, NULL};
			execve("./child", childArgv, NULL);
			break;	// compiler whines
		}
		default: {
			pids[i] = child;
			continue;
		}
		}
	}

	int status, id;
	for(size_t i = 0; i < processNum; ++i) {
		id = wait(&status);
	}

	pthread_mutex_destroy(mut);

	if(shm_unlink(SHARED_RESULT) == -1) {
		fprintf(stderr, "Failed to unlink res\n");
		fprintf(stderr, "ERRNO = %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if(shm_unlink(SHARED_MUTEX) == -1) {
		fprintf(stderr, "Failed to unlink mut\n");
		fprintf(stderr, "ERRNO = %d\n", errno);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "P> result = %d\n", *res);

	exit(EXIT_SUCCESS);
}
