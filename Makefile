make: start run

start: main.c
	gcc -o main -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast main.c -lpthread

run: main
	./main -n 4 -p 0.75 -q 5 -t 3 -b 0.05
