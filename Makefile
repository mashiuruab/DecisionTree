all: run

run: test.cpp
	g++ -g -o run -std=c++11 test.cpp

clean:
	-rm run
