all: main
main: main.o
	gcc -ggdb3 main.o -o main
main.o: main.c
	gcc -ggdb3 -c main.c
clean:
	rm -rf *o main
