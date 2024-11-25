#include <stdio.h>

#pragma pack(push, 1)
struct City {
	char name[100];
	long double latitude;
	long double longitude;
};
#pragma pack(pop)

union Serialized {
	struct City city;
	unsigned char buffer[sizeof(struct City)];
};

FILE* fio(const char* filepath, const char* mode);

unsigned char* read_char(const char* filepath, size_t size);
unsigned char* read_bin(const char* filepath, size_t size);
unsigned char* read_64(const char* filepath, size_t size);

unsigned char code(unsigned char symbol);
void process(unsigned char* buffer, size_t size, union Serialized* un,
			 unsigned char (*op)(unsigned char symbol));
