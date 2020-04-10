all:
	g++ -c test.cpp
	g++ test.o -o test -lsfml-graphics -lsfml-window -lsfml-system
	rm test.o

run:
	./test

clean:
	rm test
	rm *.o
