.DEFAULT_GOAL := compile

compile:
	clang++ -Wall -std=c++11 -I../lib -I../lib/bogaudio/dsp -c test.cpp ../lib/bogaudio/dsp/*.cpp ../lib/bogaudio/dsp/filters/*.cpp
	clang++ -Wall -std=c++11 *.o -o run-test

clean:
	rm -f *.o
	rm -f run-test
	rm -f out.*
