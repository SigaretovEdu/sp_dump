#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern void find_num(uint64_t size, int32_t* h, int32_t* w, int* buffer);

int main(int argc, char* argv[]) {
	printf("Enter size of array:\n(h w) = ");
	int32_t h, w;
	if(scanf("%d %d", &h, &w) != 2) {
		printf("You did something wrong...\n");
		exit(1);
	}
	if(w <= 0 || h <= 0) {
		printf("Wrong size values\n");
		exit(0);
	}
	uint64_t size = w * h;

	int* buffer = (int*)calloc(w * h, sizeof(int32_t));
	if(!buffer) {
		printf("Failed to initialize buffer\n");
		exit(1);
	}

	if(argc == 2 && strcmp(argv[1], "-r") == 0) {
		int* curr = buffer;
		srand(time(NULL));
		for(uint64_t i = 0; i < size; ++i) {
			*curr = rand() - RAND_MAX / 2;
			++curr;
		}
	}
	else if(argc == 1) {
		printf("Please enter 32bit values of array %dx%d:\n", h, w);
		int* curr = buffer;
		for(uint64_t i = 0; i < size; ++i) {
			if(scanf("%d", curr) != 1) {
				printf("Failed to read value\n");
				exit(1);
			}
			++curr;
		}
	}
	else {
		printf("Unknown arguments\n");
		exit(1);
	}

	printf("Your array:\n");
	for(size_t i = 0; i < size; ++i) {
		printf("%d ", *(buffer + i));
		if((i + 1) % w == 0) {
			printf("\n");
		}
	}
	printf("\n");
	int y = h, x = w;
	find_num(size, &y, &x, buffer);
	printf("minval(%d) at (%d, %d)\n", buffer[w * y + x], y + 1, x + 1);

	free(buffer);

	return 0;
}
