main: main_generated.c api.h
	gcc main_generated.c -o main -O3 -std=c11 -w -Wall

validator/validator: validator/validator.cpp
	g++ validator/validator.cpp -o validator/validator -O3

main_generated.c: main.c variables.c combine.py
	python3 combine.py

.PHONY=run
run: main validator/validator
	./main < testdata/test.in | validator/validator

test: main
	./main < testdata/test.in > out
