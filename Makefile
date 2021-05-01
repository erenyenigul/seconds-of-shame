make: start run

start: main.c
	gcc -o main main.c -lpthread -lm

run: main
	./main -n 5 -p 1 -q 1 -t 1 -b 1