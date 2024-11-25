NASM=nasm
NASMFLAGS=-f elf64 -g 

GAS=as
GASFLAGS=-g 

.PHONY: all
all: sol
	@rm -rf *.o

%.o : %.asm
	@$(NASM) $(NASMFLAGS) $< -o $@

%.o : %.s
	@$(GAS) $(GASFLAGS) $< -o $@

sol: sol.o
	@ld $^ -o $@
