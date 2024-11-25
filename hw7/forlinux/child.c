#include "utils.h"

void runShared(int argc, char* argv[]);
void runPipe(void);

int main(int argc, char* argv[]) {
	if(strcmp(argv[1], "--shared") == 0) {
		runShared(argc, argv);
	}
	else {
		runPipe();
	}
	exit(EXIT_SUCCESS);
}

void runShared(int argc, char* argv[]) {
	int memFD = -1, resFD = -1, mutFD = -1;
	// opening shared memory
	if((memFD = shm_open(SHMEM, O_RDWR, S_IWUSR | S_IRUSR)) == -1) {
		fprintf(stderr, "c %d> Failed to open shared memory for memFD\n");
		exit(EXIT_FAILURE);
	}
	if((resFD = shm_open(SHRES, O_RDWR, S_IWUSR | S_IRUSR)) == -1) {
		fprintf(stderr, "c %d> Failed to open shared memory for resFD\n");
		exit(EXIT_FAILURE);
	}
	if((mutFD = shm_open(SHMUT, O_RDWR, S_IWUSR | S_IRUSR)) == -1) {
		fprintf(stderr, "c %d> Failed to open shared memory for mutFD\n");
		exit(EXIT_FAILURE);
	}

	char* mem = NULL;
	int* res = NULL;
	pthread_mutex_t* mut = NULL;

	// mapping shared memory
	if((mem = mmap(NULL, atoi(argv[4]), PROT_READ, MAP_SHARED, memFD, 0)) == MAP_FAILED) {
		fprintf(stderr, "c %d> Failed to map mem\n");
		exit(EXIT_FAILURE);
	}
	if((res = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, resFD, 0)) ==
	   MAP_FAILED) {
		fprintf(stderr, "c %d> Failed to map res\n");
		exit(EXIT_FAILURE);
	}
	if((mut = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutFD, 0)) ==
	   MAP_FAILED) {
		fprintf(stderr, "c %d> Failed to map mut\n");
		exit(EXIT_FAILURE);
	}

	// closing descriptors
	close(memFD);
	close(resFD);
	close(mutFD);

	// initializing values
	size_t offset = atoi(argv[2]);
	size_t partlen = atoi(argv[3]);
	int ans = 0;

	// calculate ans
	for(size_t i = 0; i < partlen; ++i) {
		if(mem[i + offset] <= 31 || mem[i + offset] == 127) {
			++ans;
		}
	}

	pthread_mutex_lock(mut);
	*res += ans;
	pthread_mutex_unlock(mut);
	sleep(6000);
	return;
}
void runPipe(void) {
	char buff[100];
	int ans = 0;
	size_t len;
	while(1) {
		len = read(STDIN_FILENO, buff, 100);
		for(size_t i = 0; i < len; ++i) {
			if(buff[i] <= 31 || buff[i] == 127) {
				++ans;
			}
		}

		if(len != 100) {
			break;
		}
	}

	printf("%d", ans);
}
