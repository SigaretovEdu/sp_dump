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

struct City* read_from_stdin();
struct City* read_from_file(char* filepath);

unsigned char code(unsigned char symbol);
unsigned char* process(unsigned char* buffer, size_t size,
					   unsigned char (*op)(unsigned char symbol));

FILE* fio(const char* filepath, const char* mode);

void write_chars(unsigned char* buffer, size_t size);
void write_64(unsigned char* buffer, size_t size);
void write_bin(unsigned char* buffer, size_t size);
