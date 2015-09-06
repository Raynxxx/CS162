all: hello
hello: main.o fibonacci.o hello.o
	gcc main.o fibonacci.o hello.o -o hello
main.o: main.c
	gcc -c main.c
fibonacci.o: fibonacci.c
	gcc -c fibonacci.c
hello.o: hello.c
	gcc -c hello.c
clean:
	rm -rf *o hello
