make: start run

start: main.c
	gcc -o main -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast main.c -lpthread

run: main
	./main -n 5 -p 0.5 -q 3 -t 10 -b 1
