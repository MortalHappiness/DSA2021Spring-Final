main: main.c ../api.h
	gcc main.c -o main -O3 -std=c11 -w -Wall

.PHONY=run
run: main
	./main < ../testdata/test.in > ../variables.c
