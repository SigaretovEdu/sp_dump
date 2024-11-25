#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 4

int print_string(char *s);

int print_string(char *s)
{
    printf("This is a string \"%s\"", s);
    return 0;
}

int main()
{
    int len = MAX_SIZE;
    int ret = 0;
    char *c;
    
    c = NULL;
    c = (char *)malloc(sizeof(char) * len);
    
    len = strlen(c);
    strncpy(c, "Hello world", len);
    
    ret = print_string(c);
    
    free(c);
    
    return 0;
}
