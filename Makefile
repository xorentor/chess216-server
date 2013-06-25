all:
	nasm -f elf32 asm/roundup.asm -o ./roundup.o
	gcc -Wall -m32 -O2 roundup.o *.c -L./lib/ -lfiledb -lal -lpthread -o server -lm

clean:
	rm -f ./server
	rm -f ./roundup.o
