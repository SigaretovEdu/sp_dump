#include "utils.h"

void runShared(char* filepath, int filesize, int threadsNum);
void runPipe(char* filepath, int filesize, int threadsNum);

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
		runShared(filepath, filesize, threadsNum);
	}
	else {
		printf("go to pipe\n");
		runPipe(filepath, filesize, threadsNum);
	}

	exit(EXIT_SUCCESS);
}

void runShared(char* filepath, int filesize, int threadsNum) {
	int memFD = -1, resFD = -1, mutFD = -1;
	// opening shared memory
	if((memFD = shm_open(SHMEM, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR)) == -1) {
		fprintf(stderr, "Failed to open shared memory for memFD\n");
		exit(EXIT_FAILURE);
	}
	if((resFD = shm_open(SHRES, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR)) == -1) {
		fprintf(stderr, "Failed to open shared memory for resFD\n");
		exit(EXIT_FAILURE);
	}
	if((mutFD = shm_open(SHMUT, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR)) == -1) {
		fprintf(stderr, "Failed to open shared memory for mutFD\n");
		exit(EXIT_FAILURE);
	}

	// trucating shared memory
	if(ftruncate(memFD, filesize) != 0) {
		fprintf(stderr, "Failed to truncate memFD\n");
		exit(EXIT_FAILURE);
	}
	if(ftruncate(resFD, sizeof(int)) != 0) {
		fprintf(stderr, "Failed to truncate resFD\n");
		exit(EXIT_FAILURE);
	}
	if(ftruncate(mutFD, sizeof(pthread_mutex_t)) != 0) {
		fprintf(stderr, "Failed to truncate mutFD\n");
		exit(EXIT_FAILURE);
	}

	char* mem = NULL;
	int* res = NULL;
	pthread_mutex_t* mut = NULL;

	// mapping shared memory
	if((mem = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, memFD, 0)) == MAP_FAILED) {
		fprintf(stderr, "Failed to map mem\n");
		exit(EXIT_FAILURE);
	}
	if((res = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, resFD, 0)) ==
	   MAP_FAILED) {
		fprintf(stderr, "Failed to map res\n");
		exit(EXIT_FAILURE);
	}
	if((mut = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutFD, 0)) ==
	   MAP_FAILED) {
		fprintf(stderr, "Failed to map mut\n");
		exit(EXIT_FAILURE);
	}

	// closing descriptors
	close(memFD);
	close(resFD);
	close(mutFD);

	// initialize values
	*res = 0;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(mut, &attr);

	size_t partlen = filesize / threadsNum;
	size_t offset = 0;
	char p1[33];
	char p2[33];
	char p3[33];

	FILE* fp;
	if((fp = fopen(filepath, "r")) == NULL) {
		fprintf(stderr, "Failed to open %s\n", filepath);
		exit(EXIT_FAILURE);
	}
	if(fread(mem, sizeof(char), filesize, fp) != filesize) {
		fprintf(stderr, "Failed to read %s\n", filepath);
		exit(EXIT_FAILURE);
	}
	fclose(fp);

	// starting processes
	for(size_t i = 0; i < threadsNum; ++i) {
		if(i == (threadsNum - 1)) {
			partlen = filesize - (partlen * i);
		}

		sprintf(p1, "%zu", offset);
		sprintf(p2, "%zu", partlen);
		sprintf(p3, "%zu", filesize);

		offset += partlen;

		switch(fork()) {
		case -1: {
			fprintf(stderr, "Failed to create process\n");
			exit(EXIT_FAILURE);
		}
		case 0: {
			char* childArgv[] = {"./child", "--shared", p1, p2, p3, NULL};
			execve("./child", childArgv, NULL);
			break;
		}
		default: {
			break;
		}
		}
	}
	
	int status, id;
	for(size_t i = 0; i < threadsNum; ++i) {
		id = wait(&status);
	}

	pthread_mutex_destroy(mut);

	// unlinking memory
	shm_unlink(SHMEM);
	shm_unlink(SHRES);
	shm_unlink(SHMUT);

	// printing result
	printf("res = %d\n", *res);

	return;
}
void runPipe(char* filepath, int filesize, int threadsNum) {
	int* fdToChild = (int*)calloc(threadsNum, sizeof(int));
	int* fdToParent = (int*)calloc(threadsNum, sizeof(int));
	int fdc[2];
	int fdp[2];

	for(size_t i = 0; i < threadsNum; ++i) {
		pipe(fdc);
		pipe(fdp);

		switch(fork()) {
		case -1: {
			fprintf(stderr, "failed to fork\n");
			exit(EXIT_FAILURE);
			break;
		}
		case 0: {
			dup2(fdc[0], STDIN_FILENO);
			close(fdc[0]);
			close(fdc[1]);
			dup2(fdp[1], STDOUT_FILENO);
			close(fdp[0]);
			close(fdp[1]);

			char* params[] = {"./child", "--pipe", NULL};
			if(execve("./child", params, NULL) == -1) {
				fprintf(stderr, "failed to exec\n");
				exit(EXIT_FAILURE);
			}
			break;
		}
		default: {
			close(fdc[0]);
			close(fdp[1]);
			fdToChild[i] = fdc[1];
			fdToParent[i] = fdp[0];
		}
		}
	}

	size_t partlen = filesize / threadsNum;
	size_t offset = 0;

	int fp;
	if((fp = open(filepath, O_RDONLY)) == -1) {
		fprintf(stderr, "Failed to read %s\n", filepath);
	}
	for(size_t i = 0; i < threadsNum; ++i) {
		if(i == (threadsNum - 1)) {
			partlen = filesize - (partlen * i);
		}

		char* mess = (char*)calloc(partlen, sizeof(char));
		pread(fp, mess, partlen, offset);
		offset += partlen;

		// printf("to child: '");
		// for(size_t k = 0; k < partlen; ++k) {
		// 	printf("%c", mess[k]);
		// }
		// printf("'\n");

		write(fdToChild[i], mess, partlen);
		free(mess);
		mess = NULL;

		close(fdToChild[i]);
	}
	close(fp);

	char mess[100];
	int ans = 0;
	for(size_t i = 0; i < threadsNum; ++i) {
		size_t len = read(fdToParent[i], mess, 100);
		mess[len] = 0;
		// printf("from child, len = %lu: '", len);
		// for(size_t k = 0; k < len; ++k) {
		// 	printf("%c", mess[k]);
		// }
		// printf("'\n");
		ans += atoi(mess);
	}

	for(size_t i = 0; i < threadsNum; ++i) {
		close(fdToParent[i]);
	}

	int status, id;
	for(size_t i = 0; i < threadsNum; ++i) {
		id = wait(&status);
	}

	printf("ans = %d\n", ans);

	return;
}
