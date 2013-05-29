all:
	gcc -m32 -O2 *.c -L./lib/ -lfiledb -lal -lpthread -o server

clean:
	rm -f ./server
