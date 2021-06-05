main: main.c api.h
	gcc main.c -o main -O3 -std=c11 -w -Wall

validator/validator: validator/validator.cpp
	g++ validator/validator.cpp -o validator/validator -O3

.PHONY=run
run: main validator/validator
	./main < testdata/test.in | validator/validator

output: main
	./main < testdata/test.in 

test: main
	./main < testdata/test.in > out
