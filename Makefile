make: start run

start: main.c
	gcc -o main main.c -lpthread

run: main
	./main -n 5 -p 0.5 -q 3 -t 1 -b 1