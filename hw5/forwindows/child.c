#include "utils.h"

int main(int argc, char* argv[]) {
	char name[100];
	sprintf(name, "%s.out\0", argv[1]);
	int ans = 0;
	FILE* fp = fopen(argv[1], "r");
	if(fp == NULL) {
		fprintf(stderr, "Failed to read file %s from child\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	char c;
	while((c = fgetc(fp)) != EOF) {
		if(c <= 31 || c == 127) {
			++ans;
		}
	}
	fclose(fp);
	fp = fopen(name, "w");
	fprintf(fp, "%d", ans);
	fclose(fp);
	fp = NULL;
	exit(EXIT_SUCCESS);
}
