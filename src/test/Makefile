#.DEFAULT_GOAL := test
#
#compile:
#	clang++ -Wall -std=c++17 -I../../lib -I../../lib/bogaudio/dsp -c test.cpp ../../lib/bogaudio/dsp/*.cpp ../../lib/bogaudio/dsp/filters/*.cpp
#
#	clang++ -Wall -std=c++17 *.o -o test
#	rm -f *.o
#
#test: compile
#	./test
#	rm -f test

#####################################################################

.DEFAULT_GOAL := compile

compile:
	clang++ -Wall -std=c++17 -c test.cpp
	clang++ -Wall -std=c++17 *.o -o run-test

clean:
	rm -f *.o
	rm -f run-test
	rm -f out.*-test
