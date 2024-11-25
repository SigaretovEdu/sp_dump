#include "utils.h"

// ps -ef | grep sol
// ps -T -p <pid>

struct ThreadParams {
	size_t offset;
	size_t len;
};

size_t ans = 0;
char* filepath;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* calc(void* args);

int main(int argc, char* argv[]) {
	checkArguments(argc, argv);

	filepath = argv[1];
	size_t fileSize = getSize(argv[1]);
	size_t threadsNum = atoi(argv[2]);

	if((threadsNum > fileSize / 2) || (threadsNum == 0)) {
		threadsNum = fileSize / 2;
		if(threadsNum == 0 && fileSize) {
			threadsNum = 1;
		}
		fprintf(stderr, "You probably specified to much threads\n");
		fprintf(stderr, "Threads number updated to %zu\n", threadsNum);
	}

	size_t partLen = fileSize / threadsNum;
	printf("fileSize = %lu, threadsNum = %lu, partLen = %lu\n", fileSize, threadsNum, partLen);

	pthread_t* threadsIds = (pthread_t*)calloc(threadsNum, sizeof(pthread_t));
	struct ThreadParams* threadsParams =
		(struct ThreadParams*)calloc(threadsNum, sizeof(struct ThreadParams));
	for(size_t thrNum = 0; thrNum < threadsNum; ++thrNum) {
		threadsParams[thrNum].offset = thrNum * partLen;
		threadsParams[thrNum].len =
			(thrNum != threadsNum - 1) ? partLen : (fileSize - partLen * thrNum);
	}
	void* retVal;
	int success = 1;

	for(size_t thrNum = 0; thrNum < threadsNum; ++thrNum) {
		pthread_create(&threadsIds[thrNum], NULL, calc, &threadsParams[thrNum]);
	}
	for(size_t thrNum = 0; thrNum < threadsNum; ++thrNum) {
		pthread_join(threadsIds[thrNum], &retVal);
		printf("Thread %lu finished with retVal %d\n", threadsIds[thrNum], *((int*)retVal));
		success |= *((int*)retVal);
		free(retVal);
	}
	if(!success) {
		success = 1;
	}
	printf("All threads finished, success = %d\n", success);
	printf("Control symbols: %zu\n", ans);

	free(threadsIds);
	free(threadsParams);
	
	exit(EXIT_SUCCESS);
}

void* calc(void* args) {
	struct ThreadParams* params = (struct ThreadParams*)args;
	int* retVal = malloc(sizeof(int));
	*retVal = 0;

	int fd;
	if((fd = open(filepath, O_RDONLY)) == -1) {
		fprintf(stderr, "Failed to open %s in thread %lu\n", filepath, pthread_self());
		*retVal = 1;
		pthread_exit((void*)retVal);
	}

	char* buff = (char*)malloc(params->len * sizeof(char));
	pread(fd, buff, params->len, params->offset);

	int localAns = 0;
	for(size_t i = 0; i < params->len; ++i) {
		if(buff[i] <= 31 || buff[i] == 127) {
			++localAns;
		}
	}
	close(fd);
	free(buff);
	printf("\tIn thread %lu params: offset=%lu, len=%lu, localAns = %d\n", pthread_self(),
		   params->offset, params->len, localAns);

	pthread_mutex_lock(&mutex);
	ans += localAns;
	pthread_mutex_unlock(&mutex);

	pthread_exit((void*)retVal);
}
