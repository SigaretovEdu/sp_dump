#include "des.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("Not enough args, please provide args: des (--bin|--base64|--char) filepath\n");
		exit(1);
	}
	unsigned char* buffer = NULL;

	union Serialized ser;
	size_t size = sizeof(ser.city.name) + sizeof(ser.city.latitude) + sizeof(ser.city.longitude);
	assert(sizeof(struct City) == size);

	if(strcmp(argv[1], "--char") == 0) {
		buffer = read_char(argv[2], size);
	}
	else if(strcmp(argv[1], "--bin") == 0) {
		buffer = read_bin(argv[2], size);
	}
	else if(strcmp(argv[1], "--base64") == 0) {
		buffer = read_64(argv[2], size);
	}
	else {
		printf("Wrong format provided\n");
		exit(1);
	}

	process(buffer, size, &ser, code);

	printf("Name = %s\nlatitude = %Lf\nlongitude = %Lf\n", ser.city.name, ser.city.latitude,
		   ser.city.longitude);

	free(buffer);
	return 0;
}

FILE* fio(const char* filepath, const char* mode) {
	FILE* f = fopen(filepath, mode);
	if(f) {
		return f;
	}
	else {
		printf("Failed to open %s with mode %s", filepath, mode);
		exit(1);
	}
}

unsigned char* read_char(const char* filepath, size_t size) {
	FILE* in = fio(filepath, "r");
	unsigned char* buffer = malloc(size);
	for(size_t i = 0; i < size; ++i) {
		if(fscanf(in, "%c", &buffer[i]) != 1) {
			printf("Failed to read symbol on index %zu\n", i);
			exit(1);
		}
	}
	fclose(in);
	return buffer;
}

unsigned char* read_bin(const char* filepath, size_t size) {
	FILE* in = fio(filepath, "r");
	unsigned char* buffer = malloc(size);
	for(size_t i = 0; i < size; ++i) {
		unsigned char tmp = 0;
		char c;
		for(size_t pos = 0; pos < 8; ++pos) {
			if(fscanf(in, "%c", &c) != 1) {
				printf("Failed to read bin symbol from %s", filepath);
				exit(1);
			}
			if(c == '1') {
				++tmp;
			}
			if(pos != 7) {
				tmp = tmp << 1;
			}
		}
		buffer[i] = tmp;
	}
	fclose(in);

	return buffer;
}

unsigned char* read_64(const char* filepath, size_t size) {
	FILE* in = fio(filepath, "r");
	unsigned char* buffer = malloc(size);

	const char e_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	for(size_t i = 0; i < size; i += 3) {
		unsigned char pos1, pos2, pos3, pos4;
		if(fscanf(in, "%c%c%c%c", &pos1, &pos2, &pos3, &pos4) != 4) {
			printf("Failed to read base64 from %s\n", filepath);
			exit(0);
		}

		unsigned char p1 = (strchr(e_chars, pos1)) - e_chars;
		unsigned char p2 = (strchr(e_chars, pos2)) - e_chars;
		unsigned char p3 = (strchr(e_chars, pos3)) - e_chars;
		unsigned char p4 = (strchr(e_chars, pos4)) - e_chars;

		unsigned char tmp1 = (p1 << 2) + ((p2 >> 4) & 3);
		unsigned char tmp2 = ((p2 & 15) << 4) + ((p3 >> 2) & 15);
		unsigned char tmp3 = ((p3 & 3) << 6) + (p4);

		buffer[i] = tmp1;
		if(pos3 != '=') {
			buffer[i + 1] = tmp2;
		}
		if(pos4 != '=') {
			buffer[i + 2] = tmp3;
		}
	}

	fclose(in);

	return buffer;
}

unsigned char code(unsigned char symbol) {
	return (((~symbol) >> 4) << 4) + (symbol & 15);
}

void process(unsigned char* buffer, size_t size, union Serialized* un,
			 unsigned char (*op)(unsigned char symbol)) {
	for(size_t i = 0; i < size; ++i) {
		un->buffer[i] = op(buffer[i]);
	}
}
