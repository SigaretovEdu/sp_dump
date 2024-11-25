NASM=nasm
NASMFLAGS=-f elf64 -g 

GCC=gcc
GCCFLAGS=-g -no-pie

.PHONY: all
all: sol 
	@rm -rf *.o

%.o : %.asm
	@$(NASM) $(NASMFLAGS) $< -o $@

sol: main.c func.o
	@$(GCC) $(GCCFLAGS) $^ -o $@

