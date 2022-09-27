.DEFAULT_GOAL := compile

compile:
	clang++ -Wall -std=c++11 -c test.cpp
	clang++ -Wall -std=c++11 *.o -o run-test

clean:
	rm -f *.o
	rm -f run-test
	rm -f out.*
