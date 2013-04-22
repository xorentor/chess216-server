all:
	gcc -m32 -O2 *.c -std=gnu99 -lpthread -o server
