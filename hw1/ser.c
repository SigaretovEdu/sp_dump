#include "ser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	struct City* city = NULL;
	if(argc < 2 ||
	   (argc >= 2 && strcmp(argv[1], "--char") != 0 && strcmp(argv[1], "--bin") != 0 &&
		strcmp(argv[1], "--base64") != 0) ||
	   argc > 3) {
		printf("ser (--char|--bin|--base64) [filepath]\n");
		exit(1);
	}
	if(argc == 2) {
		city = read_from_stdin();
	}
	else {
		city = read_from_file(argv[2]);
	}

	if(!city) {
		fprintf(stderr, "No struct City got\n");
		exit(1);
	}
	else {
		printf("Struct city:\nname = %s\nlatitude = %Lf\nlongitude = %Lf\n", city->name,
			   city->latitude, city->longitude);
	}

	size_t buff_size = sizeof(city->name) + sizeof(city->latitude) + sizeof(city->longitude);
	assert(sizeof(struct City) == buff_size);

	union Serialized serialized_city;
	serialized_city.city = *city;

	unsigned char* res_buff = process(serialized_city.buffer, buff_size, code);

	if(strcmp(argv[1], "--char") == 0) {
		write_chars(res_buff, buff_size);
	}
	else if(strcmp(argv[1], "--bin") == 0) {
		write_bin(res_buff, buff_size);
	}
	else {
		write_64(res_buff, buff_size);
	}

	free(city);
	free(res_buff);

	return 0;
}

struct City* read_from_stdin() {
	struct City* city = malloc(sizeof(struct City));
	if(city) {
		printf("Enter city(max len = 100): ");
		if(scanf("%100s", city->name) != 1) {
			printf("no name provided\n");
			exit(1);
		}
		char c;
		if(scanf("%c", &c) != EOF && c != '\n') {
			printf("City name lenght more than 100 symbols\n");
			exit(1);
		}
		printf("Enter latitude: ");
		if(scanf("%Lf", &city->latitude) != 1) {
			printf("no latitude provided\n");
			exit(1);
		}
		printf("Enter longitude: ");
		if(scanf("%Lf", &city->longitude) != 1) {
			printf("no longitude provided\n");
			exit(1);
		}

		printf("You entered: %s %Lf %Lf\n", city->name, city->latitude, city->longitude);
	}
	return city;
}

struct City* read_from_file(char* filepath) {
	FILE* fp = fio(filepath, "r");
	struct City* city = malloc(sizeof(struct City));

	if(city) {
		if(fscanf(fp, "%100s %Lf %Lf", city->name, &city->latitude, &city->longitude) != 3) {
			printf("some data not provided\n");
			exit(1);
		}
	}

	fclose(fp);

	return city;
}

unsigned char code(unsigned char symbol) {
	return (((~symbol) >> 4) << 4) + (symbol & 15);
}

unsigned char* process(unsigned char* buffer, size_t size,
					   unsigned char (*op)(unsigned char symbol)) {
	unsigned char* res_buff = malloc(size * sizeof(unsigned char));
	for(size_t i = 0; i < size; ++i) {
		res_buff[i] = op(buffer[i]);
	}
	return res_buff;
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
	fclose(f);
}

void write_chars(unsigned char* buffer, size_t size) {
	FILE* out = fio("../serialized_struct", "w");
	fwrite(buffer, sizeof(buffer[0]), size, out);
	fclose(out);
}
void write_64(unsigned char* buffer, size_t size) {
	FILE* out = fio("../serialized_struct", "w");

	const char* e_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	for(size_t i = 0; i < size; i += 3) {
		unsigned char tmp1 = buffer[i];
		unsigned char tmp2 = (i >= size - 1) ? 0 : buffer[i + 1];
		unsigned char tmp3 = (i >= size - 2) ? 0 : buffer[i + 2];

		unsigned char pos1 = tmp1 >> 2;
		unsigned char pos2 = ((tmp1 & 3) << 4) + (tmp2 >> 4);
		unsigned char pos3 = ((tmp2 & 15) << 2) + (tmp3 >> 6);
		unsigned char pos4 = tmp3 & 63;

		fprintf(out, "%c%c%c%c", e_chars[pos1], e_chars[pos2], e_chars[pos3], e_chars[pos4]);
		if(i == size - 2) {
			fputc('=', out);
		}
		if(i == size - 1) {
			fputc('=', out);
		}
	}

	fclose(out);
}
void write_bin(unsigned char* buffer, size_t size) {
	FILE* out = fio("../serialized_struct", "w");

	for(size_t i = 0; i < size; ++i) {
		fprintf(out, "%d", buffer[i] >> 7);
		fprintf(out, "%d", (buffer[i] >> 6) & 1);
		fprintf(out, "%d", (buffer[i] >> 5) & 1);
		fprintf(out, "%d", (buffer[i] >> 4) & 1);
		fprintf(out, "%d", (buffer[i] >> 3) & 1);
		fprintf(out, "%d", (buffer[i] >> 2) & 1);
		fprintf(out, "%d", (buffer[i] >> 1) & 1);
		fprintf(out, "%d", buffer[i] & 1);
	}

	fclose(out);
}
