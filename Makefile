all: simpleshell

simpleshell: main.o
	gcc -o simpleshell main.o

main.o: main.c
	gcc -c main.c -Wall

clean:
	rm simpleshell main.o
