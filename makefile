all: main.c project1

project1: main.o
	gcc main.o -o project1

main.o: main.c
	gcc -c main.c

clean:
	rm *o project1
