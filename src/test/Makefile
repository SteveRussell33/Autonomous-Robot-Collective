.DEFAULT_GOAL := test

compile:
	clang++ -Wall -std=c++17 -c test.cpp
	clang++ -Wall -std=c++17 *.o -o test
	rm -f *.o

test: compile
	./test
	rm -f test
