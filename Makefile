make: start run

start: main.c
	gcc -o main -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast main.c -lpthread

run: main
	./main -n 5 -p 0.5 -q 100 -t 1 -b 1
